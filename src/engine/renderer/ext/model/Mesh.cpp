#include "Mesh.hpp"

namespace renderer{

Mesh::Mesh(std::vector<BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, TextureMaps textureMaps){
    BufferDescriptions descriptions{};

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(BasicMeshVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    VkVertexInputAttributeDescription attributeDescription = {};
    attributeDescription.binding = 0;

    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, pos);

    descriptions.attributeDescriptions.push_back(attributeDescription);

    attributeDescription.location = 1;
    attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, texCoord);

    descriptions.attributeDescriptions.push_back(attributeDescription);

    attributeDescription.location = 2;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, normal);

    descriptions.attributeDescriptions.push_back(attributeDescription);
    
    this->textureMaps = textureMaps;

    vtnBuffer = DataBuffer(descriptions, sizeof(BasicMeshVertex) * vertices.size(), vertices.data(),
     true, DATA_BUFFER_VERTEX_BIT);

    indexBuffer = DataBuffer({}, sizeof(uint32_t) * indices.size(), indices.data(), true,
     DATA_BUFFER_INDEX_BIT);

     indexCount = indices.size();
}

void Mesh::RecordCommandBuffer(CommandBuffer commandBuffer, VkBuffer instanceBuffer, uint32_t instanceCount){
        VkDeviceSize offsets[] = {0, 0};

        commandBuffer.BeginCommandBuffer(nullptr);
        //shader->UpdateDescriptorSet(descriptorWrite); do in primary command buffer
        //shader->Bind(buffer.GetCommandBuffer(), pipeline.GetPipelineLayout()); sam samee
        VkBuffer buffers[] = {vtnBuffer.GetBuffer(), instanceBuffer};

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.GetCommandBuffer(), indexCount, instanceCount, 0, 0, 0);
        commandBuffer.EndCommandBuffer();
}
void Mesh::RecordCommandBuffer(CommandBuffer commandBuffer){
        VkDeviceSize offsets[] = {0};

        commandBuffer.BeginCommandBuffer(nullptr);
        VkBuffer buffers[] = {vtnBuffer.GetBuffer()};

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 0, 2, buffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.GetCommandBuffer(), indexCount, 1, 0, 0, 0);
        commandBuffer.EndCommandBuffer();

}

}