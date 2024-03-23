#include "VertexBuffer.hpp"

#define MAX_FREE_COMMAND_BUFFER_COUNT 3 //probably needs to be adjusted for performance but idk

CommandBuffer VertexBuffer::primaryCommandBuffer = CommandBuffer();
std::vector<SecondaryCommandBuffer> VertexBuffer::secondaryCommandBuffers = {};

VertexBuffer::VertexBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
 std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
 void* data, bool transferToLocalDevMem){

    if(primaryCommandBuffer.GetCommandBuffer() == VK_NULL_HANDLE){
        primaryCommandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         COMMAND_BUFFER_TRANSFER_FLAG, nullptr);

         secondaryCommandBuffers.resize(MAX_FREE_COMMAND_BUFFER_COUNT);
            for(SecondaryCommandBuffer& buff : secondaryCommandBuffers){
                buff.commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
                COMMAND_BUFFER_TRANSFER_FLAG, nullptr);
            }
    }

    VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | (transferToLocalDevMem ?
     VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0));

    VkBufferCreateFlags properties = (VkBufferCreateFlagBits)(transferToLocalDevMem ?
     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0);  

   

    CreateBuffer(size, usage, properties, buffer, bufferMemory);

   if(transferToLocalDevMem){
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          stagingBuffer, stagingBufferMemory);

        void* mappedData;
        vkMapMemory(Device::GetDevice(), stagingBufferMemory, 0, size, 0, &mappedData);
        memcpy(mappedData, data, size);
        vkUnmapMemory(Device::GetDevice(), stagingBufferMemory);


        CopyFromBuffer(stagingBuffer, size);
    }else{
        void* mappedData;
        vkMapMemory(Device::GetDevice(), bufferMemory, 0, size, 0, &mappedData);
        memcpy(mappedData, data, size);
        vkUnmapMemory(Device::GetDevice(), bufferMemory);
    }




    descriptions.attributeDescriptions = attributeDescriptions;
    descriptions.bindingDescriptions = bindingDescriptions;

    useCount = new uint32_t;
    useCount[0] = 1;
}

void VertexBuffer::CopyFromBuffer(VkBuffer srcBuffer, VkDeviceSize size){
    bool foundFreeBuffer = false;
    for(SecondaryCommandBuffer& buff : secondaryCommandBuffers){
        if(buff.free){ 
            buff.free = false;
            foundFreeBuffer = true;
            secondaryCommandBuffer = &secondaryCommandBuffers[secondaryCommandBuffers.size() - 1];
            break;
        }
    }
    if(!foundFreeBuffer){
        secondaryCommandBuffers.push_back(SecondaryCommandBuffer());
        secondaryCommandBuffers[secondaryCommandBuffers.size() - 1].commandBuffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY,
         COMMAND_BUFFER_TRANSFER_FLAG, nullptr);
         secondaryCommandBuffers[secondaryCommandBuffers.size() - 1].free = false;
         secondaryCommandBuffer = &secondaryCommandBuffers[secondaryCommandBuffers.size() - 1];
    }


    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;

    secondaryCommandBuffer->commandBuffer.BeginCommandBuffer(0, &inheritanceInfo, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(secondaryCommandBuffer->commandBuffer.GetCommandBuffer(), srcBuffer, buffer, 1, &copyRegion);

    secondaryCommandBuffer->commandBuffer.EndCommandBuffer();


}

void VertexBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
 VkBuffer& buffer, VkDeviceMemory& bufferMemory){
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    
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

    

    AllocateMemory(bufferMemory, size, properties);

    vkBindBufferMemory(Device::GetDevice(), buffer, bufferMemory, 0);
}

void VertexBuffer::AllocateMemory(VkDeviceMemory& memory, size_t size,
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

}

void VertexBuffer::UpdateCommandBuffer(){

    std::vector<VkCommandBuffer> executeList;
    for(SecondaryCommandBuffer& buff : secondaryCommandBuffers){
        if(!buff.free){
            executeList.push_back(buff.commandBuffer.GetCommandBuffer());
            buff.free = true;
        }
    }

    if(executeList.size() == 0){
        return;
    }

    primaryCommandBuffer.BeginCommandBuffer(0, nullptr, (VkCommandBufferUsageFlagBits)0);
    vkCmdExecuteCommands(primaryCommandBuffer.GetCommandBuffer(), executeList.size(), executeList.data());
    primaryCommandBuffer.EndCommandBuffer();


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer rawBuff = primaryCommandBuffer.GetCommandBuffer();
    submitInfo.pCommandBuffers = &rawBuff;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.signalSemaphoreCount = 0;

    vkQueueSubmit(Device::GetTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
}

BufferDescriptions VertexBuffer::GetDescriptions(){
    return descriptions;
}
VertexBuffer::VertexBuffer(const VertexBuffer& other){
    buffer = other.buffer;
    bufferMemory = other.bufferMemory;
    useCount = other.useCount;
    useCount[0]++;
}

VertexBuffer VertexBuffer::operator=(const VertexBuffer& other){
    if(this == &other){
        return *this;
    }

    buffer = other.buffer;
    bufferMemory = other.bufferMemory;
    useCount = other.useCount;
    useCount[0]++;

    return *this;
}

VertexBuffer::~VertexBuffer(){
    useCount[0]--;
    if(useCount[0] == 0){
        vkDestroyBuffer(Device::GetDevice(), buffer, nullptr);
        vkFreeMemory(Device::GetDevice(), bufferMemory, nullptr);
        delete useCount;
    }
}

VkBuffer VertexBuffer::GetBuffer(){
    return buffer;
}