#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>

#include <vulkan/vulkan.h>

#include "Device.hpp"

typedef struct BufferDescriptions{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
} BufferDescriptions;

class VertexBuffer{
public:
    VertexBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
     std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
     VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, void* data);


    VkBuffer GetBuffer();
    BufferDescriptions GetDescriptions();


    VertexBuffer(const VertexBuffer& other);
    VertexBuffer operator=(const VertexBuffer& other);
    ~VertexBuffer();
private:
    uint32_t* useCount;

    BufferDescriptions descriptions;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
};


