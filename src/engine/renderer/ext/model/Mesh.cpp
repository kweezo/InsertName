#include "Mesh.hpp"

namespace renderer{

uint32_t __Mesh::currentThreadIndex = 0;

__Mesh::__Mesh(std::vector<__BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, __TextureMaps textureMaps){
        __VertexInputDescriptions descriptions{};

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(__BasicMeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::get<std::vector<VkVertexInputBindingDescription>>(descriptions).push_back(bindingDescription);

        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = 0;

        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(__BasicMeshVertex, pos);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);

        attributeDescription.location = 1;
        attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription.offset = offsetof(__BasicMeshVertex, texCoord);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);

        attributeDescription.location = 2;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(__BasicMeshVertex, normal);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);
    
        this->textureMaps = textureMaps;

        __DataBufferCreateInfo vtnCreateInfo{};
        vtnCreateInfo.size = sizeof(__BasicMeshVertex) * vertices.size();
        vtnCreateInfo.data = vertices.data();
        vtnCreateInfo.transferToLocalDeviceMemory = true;
        vtnCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vtnCreateInfo.threadIndex = GetCurrentThreadIndex();

        vtnBuffer = __DataBuffer(vtnCreateInfo);

        __DataBufferCreateInfo indexCreateInfo{};
        indexCreateInfo.size = sizeof(uint32_t) * indices.size();
        indexCreateInfo.data = indices.data();
        indexCreateInfo.transferToLocalDeviceMemory = true;
        indexCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        indexCreateInfo.threadIndex = GetCurrentThreadIndex();

        indexBuffer = __DataBuffer(vtnCreateInfo);

        indexBuffer = __DataBuffer(indexCreateInfo);
}

void __Mesh::RecordDrawCommands(__CommandBuffer& commandBuffer, uint32_t instanceCount){//TODO turn into secondary command buffer
        VkBuffer buffers[] = {vtnBuffer.GetBuffer(), indexBuffer.GetBuffer()};
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 0, 1, &buffers[0], &offset);
        vkCmdBindIndexBuffer(commandBuffer.GetCommandBuffer(), buffers[1], 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.GetCommandBuffer(), indexCount, 1, 0, 0, 0);
}

uint32_t __Mesh::GetCurrentThreadIndex(){
        uint32_t prev = currentThreadIndex;
        currentThreadIndex = (currentThreadIndex + 1) % std::thread::hardware_concurrency(); 

        return prev;
}

}