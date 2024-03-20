#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"


typedef struct BufferDescriptions{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
} BufferDescriptions;

class VertexBuffer{
public:
    VertexBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
     std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
     void* data, bool transferToLocalDevMem);


    VkBuffer GetBuffer();
    BufferDescriptions GetDescriptions();

    void CopyFromBuffer(VkBuffer srcBuffer, VkDeviceSize size);

    VertexBuffer(const VertexBuffer& other);
    VertexBuffer operator=(const VertexBuffer& other);
    ~VertexBuffer();
private:
    void AllocateMemory(VkDeviceMemory memory, size_t size, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
     VkDeviceMemory& bufferMemory);

    uint32_t* useCount;

    BufferDescriptions descriptions;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    static CommandBuffer primaryCommandBuffer;
    static std::vector<SecondaryCommandBuffer> secondaryCommandBuffers;
    std::shared_ptr<SecondaryCommandBuffer> secondaryCommandBuffer;
};


