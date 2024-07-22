#include "DataBuffer.hpp"

namespace renderer{

const uint32_t TARGET_STAGING_BUFFER_COUNT_PER_THREAD = 5;

const size_t FREE_STAGING_MEMORY_TRESHOLD = 1024;

std::vector<std::vector<_DataBufferStagingCommandBuferData>> _DataBuffer::stagingCommandBuffers = {};
std::list<VkDeviceMemory> _DataBuffer::stagingMemoryDeleteQueue = {};
_Fence _DataBuffer::finishedCopyingFence = {};
std::set<uint32_t> _DataBuffer::resetPoolIndexes = {};
bool _DataBuffer::anyCommandBuffersRecorded = false;

void _DataBuffer::Init(){
    CreateCommandBuffers();

    finishedCopyingFence = _Fence(false);
}

void _DataBuffer::CreateCommandBuffers(){
    uint32_t i = 0;
    stagingCommandBuffers.resize(std::thread::hardware_concurrency());
    for(std::vector<_DataBufferStagingCommandBuferData>& commandBuffers : stagingCommandBuffers){

        _CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = _CommandBufferType::DATA;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
        stagingCommandBufferInfo.threadIndex = i;

        commandBuffers.resize(TARGET_STAGING_BUFFER_COUNT_PER_THREAD);
        for(uint32_t y = 0; y < TARGET_STAGING_BUFFER_COUNT_PER_THREAD; y++){
            commandBuffers[y] = {__CommandBuffer(stagingCommandBufferInfo), true, __Semaphore()};
        }

        i = (i + 1) % std::thread::hardware_concurrency();
    }
}

void _DataBuffer::Update(){
    SubmitCommandBuffers();
    UpdateCleanup();
}

void _DataBuffer::Cleanup(){
    stagingCommandBuffers.clear();
    finishedCopyingFence.~_Fence();
}


_DataBuffer::_DataBuffer(){
}

_DataBuffer::_DataBuffer(_DataBufferCreateInfo createInfo) : createInfo(createInfo), stagingBuffer(VK_NULL_HANDLE), stagingMemory(VK_NULL_HANDLE){

    if(!_Device::DeviceMemoryFree()){
        createInfo.transferToLocalDeviceMemory = false;
    }

    if(createInfo.transferToLocalDeviceMemory){
        CreateBuffer(buffer, createInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, createInfo.size);
        AllocateMemory(memory, buffer, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CreateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, createInfo.size);
        AllocateMemory(stagingMemory, stagingBuffer, createInfo.size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        UploadDataToMemory(stagingMemory, createInfo.data, createInfo.size);

        RecordCopyCommandBuffer(createInfo.threadIndex, createInfo.size);

    }else{
        CreateBuffer(buffer, createInfo.usage, createInfo.size);
        AllocateMemory(memory, buffer, createInfo.size, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        UploadDataToMemory(memory, createInfo.data, createInfo.size);
    }

    if(!createInfo.signalSemaphore.IsInitialized()){
        createInfo.signalSemaphore = _Semaphore(_SemaphoreCreateInfo{});
    }

    useCount = std::make_shared<uint32_t>(1);
}

void _DataBuffer::UpdateData(void* data, size_t size, uint32_t threadIndex){
    if(createInfo.size != size){
        throw std::runtime_error("Size of data does not match size of buffer, you need to create a new buffer for this");
    }

    if(!createInfo.isDynamic){
        throw std::runtime_error("This buffer isn't dynamic, so it can't be changed");
    }

    if(!_Device::DeviceMemoryFree()){
        createInfo.transferToLocalDeviceMemory = false;
    }

    if(createInfo.transferToLocalDeviceMemory){
        if(stagingMemory == VK_NULL_HANDLE){
            AllocateMemory(stagingMemory, stagingBuffer, size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        UploadDataToMemory(stagingMemory, data, size);

        RecordCopyCommandBuffer(threadIndex, size);

    }else{
        UploadDataToMemory(memory, data, size);
    }
    
}

VkBuffer _DataBuffer::GetBuffer(){
    return buffer;
}

_DataBuffer::_DataBuffer(const _DataBuffer& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    buffer = other.buffer;
    memory = other.memory;
    stagingBuffer = other.stagingBuffer;
    stagingMemory = other.stagingMemory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;
}

_DataBuffer& _DataBuffer::operator=(const _DataBuffer& other) {
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    buffer = other.buffer;
    memory = other.memory;
    stagingBuffer = other.stagingBuffer;
    stagingMemory = other.stagingMemory;
    createInfo = other.createInfo;
    useCount = other.useCount;

    (*useCount.get())++;

    return *this;
}

void _DataBuffer::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkFreeMemory(_Device::GetDevice(), memory, nullptr);
        vkDestroyBuffer(_Device::GetDevice(), buffer, nullptr); 
        if(stagingBuffer != VK_NULL_HANDLE){
            vkDestroyBuffer(_Device::GetDevice(), stagingBuffer, nullptr);
        }
        if(stagingMemory != VK_NULL_HANDLE){
            vkFreeMemory(_Device::GetDevice(), stagingMemory, nullptr);
        }

        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

_DataBuffer::~_DataBuffer(){
    Destruct();
}

void _DataBuffer::CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size){
    VkBufferCreateInfo bufferInfo{};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = usage;
    bufferInfo.size = size;
    
    if(_Device::GetQueueFamilyInfo().transferFamilyFound){
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndices[] = {_Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
         _Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex};

        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    }else{
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if(vkCreateBuffer(_Device::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create buffer");
    }
}

void _DataBuffer::AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties){
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_Device::GetDevice(), buffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_Device::GetPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size ;
            allocInfo.memoryTypeIndex = i;

            if(vkAllocateMemory(_Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }

            break;
        }
    }

    vkBindBufferMemory(_Device::GetDevice(), buffer, memory, 0);
}

void _DataBuffer::UploadDataToMemory(VkDeviceMemory memory, void* data, size_t size){
    void* mappedMem;

    if(vkMapMemory(_Device::GetDevice(), memory, 0, size, 0, &mappedMem) != VK_SUCCESS){
        throw std::runtime_error("Failed to map vertex buffer memory, out of RAM?");
    }
    memcpy(mappedMem, data, size);
    vkUnmapMemory(_Device::GetDevice(), memory);
}

_CommandBuffer _DataBuffer::RetrieveFreeStagingCommandBuffer(uint32_t threadIndex, _Semaphore signalSemaphore){
    for(_DataBufferStagingCommandBuferData& commandBuffer : stagingCommandBuffers[threadIndex]){
        if(commandBuffer.free){
            commandBuffer.free = false;
            commandBuffer.signalSemaphore = signalSemaphore;
            return commandBuffer.commandBuffer;
        }
    }

    _CommandBufferCreateInfo stagingCommandBufferInfo;
    stagingCommandBufferInfo.type = _CommandBufferType::DATA;
    stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    stagingCommandBufferInfo.threadIndex = threadIndex;

    stagingCommandBuffers[threadIndex].push_back({__CommandBuffer(stagingCommandBufferInfo), false, signalSemaphore});
    return stagingCommandBuffers[threadIndex].back().commandBuffer;
}

void _DataBuffer::RecordCopyCommandBuffer(uint32_t threadIndex, size_t size){
    _CommandBuffer commandBuffer = RetrieveFreeStagingCommandBuffer(createInfo.threadIndex, createInfo.signalSemaphore);

    commandBuffer.BeginCommandBuffer(nullptr, false);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

    commandBuffer.EndCommandBuffer();

    if(createInfo.isDynamic && size > FREE_STAGING_MEMORY_TRESHOLD){
        stagingMemoryDeleteQueue.push_front(stagingMemory);
        stagingMemory = VK_NULL_HANDLE;
    }
}

void _DataBuffer::SubmitCommandBuffers(){
  if(!anyCommandBuffersRecorded){
        return;
    }

    std::vector<VkSubmitInfo> submitInfos{};


    std::vector<VkCommandBuffer> recordedCommandBuffers{};
    std::vector<VkSemaphore> signalSemaphores{};

    std::vector<_Fence> fences{};

    for(std::vector<_DataBufferStagingCommandBuferData>& commandBuffers : stagingCommandBuffers){
        for(_DataBufferStagingCommandBuferData& commandBuffer : commandBuffers){
            if(!commandBuffer.free){
                if(!commandBuffer.signalSemaphore.IsInitialized()){
                    recordedCommandBuffers.push_back(commandBuffer.commandBuffer.GetCommandBuffer());
                    signalSemaphores.push_back(commandBuffer.signalSemaphore.GetSemaphore());
                }
                else{
                    VkCommandBuffer commandBufferHandle = commandBuffer.commandBuffer.GetCommandBuffer();
                    VkSemaphore signalSemaphoreHandle = commandBuffer.signalSemaphore.GetSemaphore();

                    VkSubmitInfo submitInfo{};
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &commandBufferHandle;

                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pSignalSemaphores = &signalSemaphoreHandle;

                    submitInfos.push_back(submitInfo);
                }
            }
        }
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = recordedCommandBuffers.size();
    submitInfo.pCommandBuffers = recordedCommandBuffers.data();

    submitInfos.push_back(submitInfo);

    VkFence finishedCopyingFenceHandle = finishedCopyingFence.GetFence();

    if(vkQueueSubmit(_Device::GetTransferQueue(), 1, &submitInfo, finishedCopyingFenceHandle) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit data buffer command buffer");
    }

    vkWaitForFences(_Device::GetDevice(), 1, &finishedCopyingFenceHandle, VK_TRUE, std::numeric_limits<uint64_t>::max());
}

void _DataBuffer::UpdateCleanup(){
    for(uint32_t i : resetPoolIndexes){
        _CommandBuffer::ResetPools(_CommandBufferType::DATA, i); 
    }
    anyCommandBuffersRecorded = false;

   stagingMemoryDeleteQueue.clear();

    for(std::vector<_DataBufferStagingCommandBuferData>& commandBuffers : stagingCommandBuffers){
        commandBuffers.resize(TARGET_STAGING_BUFFER_COUNT_PER_THREAD);
        for(_DataBufferStagingCommandBuferData& commandBuffer : commandBuffers){
            commandBuffer.free = true;
        }
    }

    for(VkDeviceMemory stagingMemory : stagingMemoryDeleteQueue){
        vkFreeMemory(_Device::GetDevice(), stagingMemory, nullptr);
    }
}

void _DataBuffer::SetSignalSemaphore(_Semaphore signalSemaphore){
    createInfo.signalSemaphore = signalSemaphore; 
}

}