#include "DataBuffer.hpp"

namespace renderer{

const uint32_t TARGET_STAGING_BUFFER_COUNT_PER_THREAD = 5;

__CommandBuffer __DataBuffer::primaryCommandBuffer = {};
std::vector<std::vector<std::pair<__CommandBuffer, bool>>> __DataBuffer::stagingCommandBuffers = {};
std::vector<std::pair<VkBuffer, VkDeviceMemory>> __DataBuffer::stagingBufferAndMemoryDeleteQueue = {};
__Fence __DataBuffer::finishedCopyingFence = {};

void __DataBuffer::Init(){
    CreateCommandBuffers();

    finishedCopyingFence = __Fence(false);
}

void __DataBuffer::CreateCommandBuffers(){
    __CommandBufferCreateInfo commandBufferInfo{};
    commandBufferInfo.type = __CommandBufferType::GENERIC;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    commandBufferInfo.threadIndex = 0;

    primaryCommandBuffer = __CommandBuffer(commandBufferInfo);

    uint32_t i = 0;
    stagingCommandBuffers.resize(std::thread::hardware_concurrency());
    for(std::vector<std::pair<__CommandBuffer, bool>>& commandBuffers : stagingCommandBuffers){

        __CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = __CommandBufferType::DATA;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
        stagingCommandBufferInfo.threadIndex = i;

        commandBuffers = std::vector<std::pair<__CommandBuffer, bool>>(TARGET_STAGING_BUFFER_COUNT_PER_THREAD, {__CommandBuffer(stagingCommandBufferInfo), false});

        i = (i + 1) % std::thread::hardware_concurrency();
    }
}

void __DataBuffer::Update(){
    RecordPrimaryCommandBuffer();
    SubmitPrimaryCommandBuffer();
    UpdateCleanup();
}

void __DataBuffer::Cleanup(){
    primaryCommandBuffer.~__CommandBuffer();
    stagingCommandBuffers.clear();
}


__DataBuffer::__DataBuffer(){
    useCount = std::make_shared<uint32_t>(1);
}

__DataBuffer::__DataBuffer(__DataBufferCreateInfo createInfo) : transferToLocalDeviceMemory(createInfo.transferToLocalDeviceMemory),
 size(createInfo.size){

    if(!__Device::DeviceMemoryFree()){
        createInfo.transferToLocalDeviceMemory = false;
    }

    if(createInfo.transferToLocalDeviceMemory){
        CreateBuffer(buffer, createInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
        AllocateMemory(memory, buffer, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CreateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
        AllocateMemory(stagingMemory, stagingBuffer, size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        UploadDataToBuffer(stagingMemory, createInfo.data, createInfo.size);

        RecordCopyCommandBuffer(createInfo.threadIndex, size);

    }else{
        CreateBuffer(buffer, createInfo.usage, size);
        AllocateMemory(memory, buffer, size, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        UploadDataToBuffer(memory, createInfo.data, createInfo.size);
    }
    
   
    useCount = std::make_shared<uint32_t>(1);
}

void __DataBuffer::UpdateData(void* data, size_t size, uint32_t threadIndex){
    if(this->size != size){
        throw std::runtime_error("Size of data does not match size of buffer, you need to create a new buffer for this");
    }

    if(!__Device::DeviceMemoryFree()){
        transferToLocalDeviceMemory = false;
    }

    if(transferToLocalDeviceMemory){
        UploadDataToBuffer(stagingMemory, data, size);

        RecordCopyCommandBuffer(threadIndex, size);

    }else{
        UploadDataToBuffer(memory, data, size);
    }
    
}

VkBuffer __DataBuffer::GetBuffer(){
    return buffer;
}

__DataBuffer::__DataBuffer(const __DataBuffer& other){
    buffer = other.buffer;
    memory = other.memory;
    stagingBuffer = other.stagingBuffer;
    stagingMemory = other.stagingMemory;
    useCount = other.useCount;

    (*useCount.get())++;
}

__DataBuffer __DataBuffer::operator=(const __DataBuffer& other) {
    if(this == &other){
        return *this;
    }


    buffer = other.buffer;
    memory = other.memory;
    stagingBuffer = other.stagingBuffer;
    stagingMemory = other.stagingMemory;
    useCount = other.useCount;

    (*useCount.get())++;

    return *this;
}

__DataBuffer::~__DataBuffer(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkFreeMemory(__Device::GetDevice(), memory, nullptr);
        vkDestroyBuffer(__Device::GetDevice(), buffer, nullptr); 
        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

void __DataBuffer::CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size){
    VkBufferCreateInfo bufferInfo{};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = usage;
    bufferInfo.size = size;
    
    if(__Device::GetQueueFamilyInfo().transferFamilyFound){
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndices[] = {__Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
         __Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex};

        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    }else{
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if(vkCreateBuffer(__Device::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create buffer");
    }
}

void __DataBuffer::AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties){
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(__Device::GetDevice(), buffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(__Device::GetPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size ;
            allocInfo.memoryTypeIndex = i;

            if(vkAllocateMemory(__Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }

            break;
        }
    }

    vkBindBufferMemory(__Device::GetDevice(), buffer, memory, 0);
}

void __DataBuffer::UploadDataToBuffer(VkDeviceMemory memory, void* data, size_t size){
    void* mappedMem;

    if(vkMapMemory(__Device::GetDevice(), memory, 0, size, 0, &mappedMem) != VK_SUCCESS){
        throw std::runtime_error("Failed to map vertex buffer memory, out of RAM?");
    }
    memcpy(mappedMem, data, size);
    vkUnmapMemory(__Device::GetDevice(), memory);
}

__CommandBuffer __DataBuffer::RetrieveFreeStagingCommandBuffer(uint32_t threadIndex){
    for(std::pair<__CommandBuffer, bool>& commandBuffer : stagingCommandBuffers[threadIndex]){
        if(std::get<bool>(commandBuffer)){
            std::get<bool>(commandBuffer) = false;
            return std::get<__CommandBuffer>(commandBuffer);
        }
    }

    __CommandBufferCreateInfo stagingCommandBufferInfo;
    stagingCommandBufferInfo.type = __CommandBufferType::DATA;
    stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
    stagingCommandBufferInfo.threadIndex = threadIndex;

    stagingCommandBuffers[threadIndex].push_back({__CommandBuffer(stagingCommandBufferInfo), false});
    return std::get<__CommandBuffer>(stagingCommandBuffers[threadIndex].back());
}

void __DataBuffer::RecordCopyCommandBuffer(uint32_t threadIndex, size_t size){
    __CommandBuffer commandBuffer = RetrieveFreeStagingCommandBuffer(threadIndex);

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;


    commandBuffer.BeginCommandBuffer(&inheritanceInfo, false);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

    commandBuffer.EndCommandBuffer();


    stagingBufferAndMemoryDeleteQueue.push_back({stagingBuffer, stagingMemory});
}

void __DataBuffer::RecordPrimaryCommandBuffer(){
    std::vector<VkCommandBuffer> usedSecondaryCommandBuffers;

    for(std::vector<std::pair<__CommandBuffer, bool>>& commandBuffers : stagingCommandBuffers){
        for(std::pair<__CommandBuffer, bool>& commandBuffer : commandBuffers){
            if(!std::get<bool>(commandBuffer)){
                usedSecondaryCommandBuffers.push_back(std::get<__CommandBuffer>(commandBuffer).GetCommandBuffer());
            }
        }
    }

    primaryCommandBuffer.BeginCommandBuffer(nullptr, false);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), usedSecondaryCommandBuffers.size(), usedSecondaryCommandBuffers.data());
    primaryCommandBuffer.EndCommandBuffer();
}

void __DataBuffer::SubmitPrimaryCommandBuffer(){
    VkCommandBuffer primaryCommandBufferRaw = primaryCommandBuffer.GetCommandBuffer();
    VkFence finishedCopyingFenceRaw = finishedCopyingFence.GetFence();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &primaryCommandBufferRaw;

    if(vkQueueSubmit(__Device::GetTransferQueue(), 1, &submitInfo, finishedCopyingFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit data buffer command buffer");
    }

    vkWaitForFences(__Device::GetDevice(), 1, &finishedCopyingFenceRaw, VK_TRUE, std::numeric_limits<uint64_t>::max());
}


void __DataBuffer::UpdateCleanup(){
   __CommandBuffer::ResetPools(__CommandBufferType::DATA); 

   for(std::pair<VkBuffer, VkDeviceMemory>& stagingBufferAndMemory : stagingBufferAndMemoryDeleteQueue){
        vkFreeMemory(__Device::GetDevice(), std::get<VkDeviceMemory>(stagingBufferAndMemory), nullptr);
        vkDestroyBuffer(__Device::GetDevice(), std::get<VkBuffer>(stagingBufferAndMemory), nullptr);
   }
   stagingBufferAndMemoryDeleteQueue.clear();

    for(std::vector<std::pair<__CommandBuffer, bool>>& commandBuffers : stagingCommandBuffers){
        commandBuffers.resize(TARGET_STAGING_BUFFER_COUNT_PER_THREAD);
        for(std::pair<__CommandBuffer, bool>& commandBuffer : commandBuffers){
            std::get<bool>(commandBuffer) = true;
        }
    }
}

}