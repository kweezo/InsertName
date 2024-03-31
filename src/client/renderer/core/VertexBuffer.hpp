#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Buffer.hpp"
#include "Fence.hpp"


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

class VertexBuffer : public Buffer{
public:
    VertexBuffer(std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
     std::vector<VkVertexInputBindingDescription> bindingDescriptions, size_t size,
     void* data, bool transferToLocalDevMem);


    BufferDescriptions GetDescriptions();

    void CopyFromBuffer(StagingBufferCopyCMDInfo stagingBuffer, VkDeviceSize size);

    static void UpdateCommandBuffer();

    static void Cleanup();

    VertexBuffer(const VertexBuffer& other);
    VertexBuffer operator=(const VertexBuffer& other);
    ~VertexBuffer();
private:
    static void CreateStagingBuffers();

    static std::vector<StagingBufferCopyCMDInfo> stagingBuffers;
    static bool createdStagingBuffers;

    static CommandBuffer commandBuffer;
    static Fence finishedCopyingFence;

    uint32_t* useCount;


    BufferDescriptions descriptions;

};


