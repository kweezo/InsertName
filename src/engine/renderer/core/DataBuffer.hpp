#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <utility>
#include <thread>
#include <limits>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"


namespace renderer{

struct DataBufferCreateInfo{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions; 

    void* data;
    size_t size;

    VkBufferUsageFlags usage;
    bool transferToLocalDeviceMemory;
    uint32_t threadIndex;
};

class DataBuffer{
public:
    static void Init();
    static void Update();
    static void Cleanup();


    DataBuffer();
    DataBuffer(DataBufferCreateInfo createInfo);

    DataBuffer(const DataBuffer& other);
    DataBuffer operator=(const DataBuffer& other);    

    ~DataBuffer();


    VkBuffer GetBuffer();
private:

    static void CreateCommandBuffers();
    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void UploadDataToBuffer(VkDeviceMemory memory, void* data, size_t size);
    static CommandBuffer RetrieveFreeStagingCommandBuffer(uint32_t threadIndex);

    static void RecordPrimaryCommandBuffer();
    static void SubmitPrimaryCommandBuffer();
    static void UpdateCleanup();

    void RecordCopyCommandBuffer(uint32_t threadIndex, size_t size);


    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    static CommandBuffer primaryCommandBuffer;
    static std::vector<std::vector<std::pair<CommandBuffer, bool>>> stagingCommandBuffers;
    static std::vector<std::pair<VkBuffer, VkDeviceMemory>> stagingBufferAndMemoryDeleteQueue;
    static Fence finishedCopyingFence;

    std::shared_ptr<uint32_t> useCount;
};

}