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
#define DATA_BUFFER_UNIFORM_BIT 4

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
   // std::vector
} BufferDescriptions;

class DataBuffer{
public:
    DataBuffer();
    DataBuffer(BufferDescriptions bufferDescriptions, size_t size,
     void* data, bool transferToLocalDevMem, uint32_t flags);

    static void LoadDataIntoImage(VkImage image, VkDeviceMemory imageMemory, size_t size, void* data);

    VkBuffer GetBuffer();

    BufferDescriptions GetDescriptions();

    void CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size);
    void UpdateData(void* data, size_t size);

    static void CopyBufferData(VkBuffer dst, void* data, size_t size); // is this needed(obligatory suicide joke)?
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

    static StagingBufferCopyCMDInfo GetStagingBuffer(size_t size);
    
    VkBuffer buff;
    VkDeviceMemory mem;

    BufferDescriptions descriptions;

    size_t size;
    bool transferToLocalDevMem;

};

}