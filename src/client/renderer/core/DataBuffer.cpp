#include "DataBuffer.hpp"

#define MAX_FREE_COMMAND_BUFFER_COUNT 3 //probably needs to be adjusted for performance but idk

namespace renderer{

std::vector<StagingBufferCopyCMDInfo> DataBuffer::stagingBuffers = {};
bool DataBuffer::createdStagingBuffers = false;
CommandBuffer DataBuffer::commandBuffer = CommandBuffer();
Fence DataBuffer::finishedCopyingFence = Fence();

DataBuffer::DataBuffer(){}

DataBuffer::DataBuffer(BufferDescriptions bufferDescriptions, size_t size,
 void* data, bool transferToLocalDevMem, uint32_t flags){

    if(!createdStagingBuffers){
        CreateStagingBuffers();

        commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_TRANSFER_FLAG, nullptr);

        finishedCopyingFence = Fence(false);
    }

    VkBufferUsageFlagBits bufferType = (flags & DATA_BUFFER_VERTEX_BIT) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if(transferToLocalDevMem){
        CreateBuffer(buff, bufferType | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
        AllocateMemory(mem, buff, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        StagingBufferCopyCMDInfo stagingBuffer = GetStagingBuffer(size);

        void *stagingData;
        if(vkMapMemory(Device::GetDevice(), stagingBuffer.bufferMemory, 0, size, 0, &stagingData) != VK_SUCCESS){
            throw std::runtime_error("Failed to map vertex buffer memory");
        }
        memcpy(stagingData, data, size);
        vkUnmapMemory(Device::GetDevice(), stagingBuffer.bufferMemory);
        

        CopyFromBuffer(stagingBuffer, size);
    }
    else{
        CreateBuffer(buff, bufferType, size);
        AllocateMemory(mem, buff, size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *stagingData;
        if(vkMapMemory(Device::GetDevice(), mem, 0, size, 0, &stagingData) != VK_SUCCESS){
            throw std::runtime_error("Failed to map vertex buffer memory");
        }
        memcpy(stagingData, data, size);
        vkUnmapMemory(Device::GetDevice(), mem);
    }


    descriptions = bufferDescriptions;

    useCount = new uint32_t;
    useCount[0] = 1;

    this->size = size;
    this->transferToLocalDevMem = transferToLocalDevMem;
}

StagingBufferCopyCMDInfo DataBuffer::GetStagingBuffer(size_t size){
        StagingBufferCopyCMDInfo *stagingBuffer;
        bool foundFreeBuff = false;
        for(StagingBufferCopyCMDInfo& currStagingBuffer : stagingBuffers){
            if(currStagingBuffer.free){
                currStagingBuffer.free = false;
                foundFreeBuff = true;
                stagingBuffer = &currStagingBuffer;
                CreateBuffer(stagingBuffer->buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
                AllocateMemory(stagingBuffer->bufferMemory, stagingBuffer->buffer, size,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                break;
            }
        }
        if(!foundFreeBuff){
            StagingBufferCopyCMDInfo tmpStagingBuffer;
            tmpStagingBuffer.free = false;
            tmpStagingBuffer.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, COMMAND_BUFFER_TRANSFER_FLAG, nullptr);
        
            CreateBuffer(tmpStagingBuffer.buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
            AllocateMemory(stagingBuffer->bufferMemory, stagingBuffer->buffer, size,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            stagingBuffers.push_back(tmpStagingBuffer);

            stagingBuffer = &stagingBuffers[stagingBuffers.size() - 1];
        }

    return *stagingBuffer;
}

void DataBuffer::CreateStagingBuffers(){
    for(int i = 0; i < MAX_FREE_COMMAND_BUFFER_COUNT; i++){
        StagingBufferCopyCMDInfo stagingBuffer;
        stagingBuffer.free = true;
        stagingBuffer.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,//one time usage
         COMMAND_BUFFER_TRANSFER_FLAG , nullptr);
        
        stagingBuffers.push_back(stagingBuffer);
    }
    createdStagingBuffers = true;
}

void DataBuffer::UpdateData(void* data, size_t size){
    if(this->size == size){
        if(transferToLocalDevMem){
            StagingBufferCopyCMDInfo stagingBuffer = GetStagingBuffer(size);

            void *stagingData;
            if(vkMapMemory(Device::GetDevice(), stagingBuffer.bufferMemory, 0, size, 0, &stagingData) != VK_SUCCESS){
                throw std::runtime_error("Failed to map vertex buffer memory");
            }
            memcpy(stagingData, data, size);
            vkUnmapMemory(Device::GetDevice(), stagingBuffer.bufferMemory);

            CopyFromBuffer(stagingBuffer, size);
        }else{
            void *mappedData;
            if(vkMapMemory(Device::GetDevice(), mem, 0, size, 0, &mappedData) != VK_SUCCESS){
                throw std::runtime_error("Failed to map vertex buffer memory");
            }
        }
    }
    else{
        throw std::runtime_error("Size of data does not match size of buffer, you need to create a new buffer for this");
    }
}

void DataBuffer::CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size){
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    stagingBuffer.commandBuffer.BeginCommandBuffer(0, &inheritanceInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(stagingBuffer.commandBuffer.GetCommandBuffer(), stagingBuffer.buffer, buff, 1, &copyRegion);

    stagingBuffer.commandBuffer.EndCommandBuffer();
}


void DataBuffer::UpdateCommandBuffer(){
    std::vector<uint32_t> cleanupList;
    std::vector<VkCommandBuffer> commandBuffers;
    for(int i = 0; i < stagingBuffers.size(); i++){
        if(!stagingBuffers[i].free){
            cleanupList.push_back(i);
            commandBuffers.push_back(stagingBuffers[i].commandBuffer.GetCommandBuffer());
        }
    }
    
    if(commandBuffers.empty()){
        return;
    }

    VkFence fence = finishedCopyingFence.GetFence();

    vkResetFences(Device::GetDevice(), 1, &fence);

    commandBuffer.BeginCommandBuffer(0, nullptr);
    vkCmdExecuteCommands(commandBuffer.GetCommandBuffer(), commandBuffers.size(), commandBuffers.data());
    commandBuffer.EndCommandBuffer();



    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer commandBufferHandle = commandBuffer.GetCommandBuffer();
    submitInfo.pCommandBuffers = &commandBufferHandle;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;

    if(vkQueueSubmit(Device::GetTransferQueue(), 1, &submitInfo, finishedCopyingFence.GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit transfer command buffer");
    }
    
    VkFence fences[] = {finishedCopyingFence.GetFence()};
    vkWaitForFences(Device::GetDevice(), 1, fences, VK_TRUE, UINT64_MAX);

    for(uint32_t i : cleanupList){
        stagingBuffers[i].free = true;
        vkDestroyBuffer(Device::GetDevice(), stagingBuffers[i].buffer, nullptr);
        vkFreeMemory(Device::GetDevice(), stagingBuffers[i].bufferMemory, nullptr);
    }

    while(stagingBuffers.size() > MAX_FREE_COMMAND_BUFFER_COUNT){
        stagingBuffers.erase(stagingBuffers.begin());
    }
}

void DataBuffer::CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size){
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = usage;
    bufferInfo.size = size;
    
    if(Device::GetQueueFamilyInfo().transferFamilyFound){
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = {Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
         Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex};
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    }else{
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if(vkCreateBuffer(Device::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create vertex buffer");
    }
}

void DataBuffer::AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size,
 VkMemoryPropertyFlags properties){
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Device::GetDevice(), buffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(Device::GetPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
        if((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size ;
            allocInfo.memoryTypeIndex = i;

            if(vkAllocateMemory(Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS){
                throw std::runtime_error("Failed to allocate vertex buffer memory");
            }

            break;
        }
    }

    vkBindBufferMemory(Device::GetDevice(), buffer, memory, 0);
}

VkBuffer DataBuffer::GetBuffer(){
    return buff;
}


void DataBuffer::Cleanup(){
    for(StagingBufferCopyCMDInfo& stagingBuffer : stagingBuffers){
        if(!stagingBuffer.free){
            stagingBuffer.free = true;
            vkDestroyBuffer(Device::GetDevice(), stagingBuffer.buffer, nullptr);
            vkFreeMemory(Device::GetDevice(), stagingBuffer.bufferMemory, nullptr);
        }
    }
    stagingBuffers.clear();
    finishedCopyingFence.~Fence();
}

DataBuffer::DataBuffer(const DataBuffer& other){
    buff = other.buff;
    mem = other.mem;
    descriptions = other.descriptions;
    useCount = other.useCount;
    useCount[0]++;
}

DataBuffer DataBuffer::operator=(const DataBuffer& other){
    if(this == &other){
        return *this;
    }

    buff = other.buff;
    mem = other.mem;
    descriptions = other.descriptions;
    useCount = other.useCount;
    useCount[0]++;

    return *this;
}

DataBuffer::~DataBuffer(){
    useCount[0]--;
    if(useCount[0] == 0){
        vkDestroyBuffer(Device::GetDevice(), buff, nullptr);
        vkFreeMemory(Device::GetDevice(), mem, nullptr);
        delete useCount;
    }
}


BufferDescriptions DataBuffer::GetDescriptions(){
    return descriptions;
}

}