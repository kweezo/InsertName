#include "Mesh.hpp"

namespace renderer{

uint32_t i_Mesh::currentThreadIndex = 0;

i_Mesh::i_Mesh(std::vector<i_BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, const i_TextureMaps& textureMaps){
        i_VertexInputDescriptions descriptions{};

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(i_BasicMeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::get<std::vector<VkVertexInputBindingDescription>>(descriptions).push_back(bindingDescription);

        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = 0;

        attributeDescription.location = 0;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(i_BasicMeshVertex, pos);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);

        attributeDescription.location = 1;
        attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription.offset = offsetof(i_BasicMeshVertex, texCoord);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);

        attributeDescription.location = 2;
        attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription.offset = offsetof(i_BasicMeshVertex, normal);

        std::get<std::vector<VkVertexInputAttributeDescription>>(descriptions).push_back(attributeDescription);
    
        this->textureMaps = textureMaps;

        i_DataBufferCreateInfo vtnCreateInfo{};
        vtnCreateInfo.size = sizeof(i_BasicMeshVertex) * vertices.size();
        vtnCreateInfo.data = vertices.data();
        vtnCreateInfo.transferToLocalDeviceMemory = true;
        vtnCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vtnCreateInfo.threadIndex = GetCurrentThreadIndex();
        vtnCreateInfo.isDynamic = false;

        i_DataBufferCreateInfo indexCreateInfo{};
        indexCreateInfo.size = sizeof(uint32_t) * indices.size();
        indexCreateInfo.data = indices.data();
        indexCreateInfo.transferToLocalDeviceMemory = true;
        indexCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        indexCreateInfo.threadIndex = GetCurrentThreadIndex();
        indexCreateInfo.isDynamic = false;

        vtnBuffer = i_DataBuffer(vtnCreateInfo);

        indexBuffer = i_DataBuffer(indexCreateInfo);


        indexCount = indices.size();
}

void i_Mesh::RecordDrawCommands(i_CommandBuffer& commandBuffer, uint32_t instanceCount){//TODO turn into secondary command buffer
        VkBuffer buffers[] = {vtnBuffer.GetBuffer(), indexBuffer.GetBuffer()};
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 0, 1, &buffers[0], &offset);
        vkCmdBindIndexBuffer(commandBuffer.GetCommandBuffer(), buffers[1], 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.GetCommandBuffer(), indexCount, 1, 0, 0, 0);
}

uint32_t i_Mesh::GetCurrentThreadIndex(){
        uint32_t prev = currentThreadIndex;
        currentThreadIndex = (currentThreadIndex + 1) % std::thread::hardware_concurrency(); 

        return prev;
}

}