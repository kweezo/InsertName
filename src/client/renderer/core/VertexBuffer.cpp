#include "VertexBuffer.hpp"

#define MAX_FREE_COMMAND_BUFFER_COUNT 3 //probably needs to be adjusted for performance but idk

std::vector<StagingBufferCopyCMDInfo> VertexBuffer::stagingBuffers = {};
bool VertexBuffer::createdStagingBuffers = false;
CommandBuffer VertexBuffer::commandBuffer = CommandBuffer();
VkFence VertexBuffer::finishedCopyingFence = VK_NULL_HANDLE;

VertexBuffer::VertexBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
 std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
 void* data, bool transferToLocalDevMem){

    if(!createdStagingBuffers){
        CreateStagingBuffers();

        commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_TRANSFER_FLAG, nullptr);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        if(vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &finishedCopyingFence) != VK_SUCCESS){
            throw std::runtime_error("Failed to create transfer command buffer fence");
        }
    }

    if(transferToLocalDevMem){
        CreateBuffer(buff, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
        AllocateMemory(mem, buff, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        StagingBufferCopyCMDInfo *stagingBuffer;

        bool foundFreeBuff = false;
        for(StagingBufferCopyCMDInfo& currStagingBuffer : stagingBuffers){
            if(currStagingBuffer.free){
                currStagingBuffer.free = false;
                foundFreeBuff = true;
                stagingBuffer = &currStagingBuffer;
                CreateBuffer(stagingBuffer->buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);

                break;
            }
        }
        if(!foundFreeBuff){
            StagingBufferCopyCMDInfo tmpStagingBuffer;
            tmpStagingBuffer.free = true;
            tmpStagingBuffer.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, COMMAND_BUFFER_TRANSFER_FLAG, nullptr);
        
            CreateBuffer(tmpStagingBuffer.buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);

            stagingBuffers.push_back(tmpStagingBuffer);

            stagingBuffer = &stagingBuffers[stagingBuffers.size() - 1];
        }

        AllocateMemory(stagingBuffer->bufferMemory, stagingBuffer->buffer, size,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *stagingData;
        if(vkMapMemory(Device::GetDevice(), stagingBuffer->bufferMemory, 0, size, 0, &stagingData) != VK_SUCCESS){
            throw std::runtime_error("Failed to map vertex buffer memory");
        }
        memcpy(stagingData, data, size);
        vkUnmapMemory(Device::GetDevice(), stagingBuffer->bufferMemory);
        

        CopyFromBuffer(stagingBuffer[0], size);
    }
    else{
        CreateBuffer(buff, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
        AllocateMemory(mem, buff, size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *stagingData;
        if(vkMapMemory(Device::GetDevice(), mem, 0, size, 0, &stagingData) != VK_SUCCESS){
            throw std::runtime_error("Failed to map vertex buffer memory");
        }
        memcpy(stagingData, data, size);
        vkUnmapMemory(Device::GetDevice(), mem);
    }


    descriptions = {attributeDescriptions, bindingDescriptions};

    useCount = new uint32_t;
    useCount[0] = 1;
}

void VertexBuffer::CreateStagingBuffers(){
    for(int i = 0; i < MAX_FREE_COMMAND_BUFFER_COUNT; i++){
        StagingBufferCopyCMDInfo stagingBuffer;
        stagingBuffer.free = true;
        stagingBuffer.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
         COMMAND_BUFFER_TRANSFER_FLAG | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr);
        
        stagingBuffers.push_back(stagingBuffer);
    }
    createdStagingBuffers = true;
}

void VertexBuffer::CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size){
    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

    stagingBuffer.commandBuffer.BeginCommandBuffer(0, &inheritanceInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(stagingBuffer.commandBuffer.GetCommandBuffer(), stagingBuffer.buffer, buff, 1, &copyRegion);

    stagingBuffer.commandBuffer.EndCommandBuffer();
}

void VertexBuffer::CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size){
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

void VertexBuffer::AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size,
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

void VertexBuffer::UpdateCommandBuffer(){
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

    if(vkQueueSubmit(Device::GetTransferQueue(), 1, &submitInfo, finishedCopyingFence) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit transfer command buffer");
    }
    
    vkWaitForFences(Device::GetDevice(), 1, &finishedCopyingFence, VK_TRUE, UINT64_MAX);

    for(uint32_t i : cleanupList){
        stagingBuffers[i].free = true;
        vkDestroyBuffer(Device::GetDevice(), stagingBuffers[i].buffer, nullptr);
        vkFreeMemory(Device::GetDevice(), stagingBuffers[i].bufferMemory, nullptr);
    }

}

VertexBuffer::VertexBuffer(const VertexBuffer& other){
    buff = other.buff;
    mem = other.mem;
    descriptions = other.descriptions;
    useCount = other.useCount;
    useCount[0]++;
}

VertexBuffer VertexBuffer::operator=(const VertexBuffer& other){
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

VertexBuffer::~VertexBuffer(){
    useCount[0]--;
    if(useCount[0] == 0){
        vkDestroyBuffer(Device::GetDevice(), buff, nullptr);
        vkFreeMemory(Device::GetDevice(), mem, nullptr);
        delete useCount;
    }
}

VkBuffer VertexBuffer::GetBuffer(){
    return buff;
}

BufferDescriptions VertexBuffer::GetDescriptions(){
    return descriptions;
}