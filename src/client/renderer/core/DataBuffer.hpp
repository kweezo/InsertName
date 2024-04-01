#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"

#define DATA_BUFFER_VERTEX_BIT 1
#define DATA_BUFFER_INDEX_BIT 2

namespace renderer{

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

class DataBuffer{
public:
    DataBuffer();
    DataBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
     std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
     void* data, bool transferToLocalDevMem, uint32_t flags);

    VkBuffer GetBuffer();

    BufferDescriptions GetDescriptions();

    void CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size);

    static void UpdateCommandBuffer();

    static void Cleanup();

    DataBuffer(const DataBuffer& other);
    DataBuffer operator=(const DataBuffer& other);
    ~DataBuffer();
private:
    static void CreateStagingBuffers();

    static std::vector<StagingBufferCopyCMDInfo> stagingBuffers;
    static bool createdStagingBuffers;

    static CommandBuffer commandBuffer;
    static Fence finishedCopyingFence;

    uint32_t* useCount;

    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    
    VkBuffer buff;
    VkDeviceMemory mem;

    BufferDescriptions descriptions;

};

}