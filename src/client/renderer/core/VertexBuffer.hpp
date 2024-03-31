#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"


typedef struct StagingBufferCopyCMDInfo{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    CommandBuffer commandBuffer;
    bool free;
} StagingBuffer;

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

    void CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size);

    static void UpdateCommandBuffer();

    VertexBuffer(const VertexBuffer& other);
    VertexBuffer operator=(const VertexBuffer& other);
    ~VertexBuffer();
private:
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);

    static void CreateStagingBuffers();

    static std::vector<StagingBufferCopyCMDInfo> stagingBuffers;
    static bool createdStagingBuffers;

    static CommandBuffer commandBuffer;

    uint32_t* useCount;

    VkBuffer buff;
    VkDeviceMemory mem;

    BufferDescriptions descriptions;
};


