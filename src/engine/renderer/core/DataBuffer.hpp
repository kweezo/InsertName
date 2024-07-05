#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <utility>
#include <thread>
#include <limits>
#include <set>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"


namespace renderer{

struct __DataBufferCreateInfo{
    void* data;
    size_t size;

    VkBufferUsageFlags usage;
    bool transferToLocalDeviceMemory;
    uint32_t threadIndex;
};

class __DataBuffer{
public:
    static void Init();
    static void Update();
    static void Cleanup();


    __DataBuffer();
    __DataBuffer(__DataBufferCreateInfo createInfo);

    __DataBuffer(const __DataBuffer& other);
    __DataBuffer operator=(const __DataBuffer& other);    

    ~__DataBuffer();

    void UpdateData(void* data, size_t size, uint32_t threadIndex);

    VkBuffer GetBuffer();
private:

    static void CreateCommandBuffers();
    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void UploadDataToBuffer(VkDeviceMemory memory, void* data, size_t size);
    static __CommandBuffer RetrieveFreeStagingCommandBuffer(uint32_t threadIndex);

    static void RecordPrimaryCommandBuffer();
    static void SubmitPrimaryCommandBuffer();
    static void UpdateCleanup();

    void RecordCopyCommandBuffer(uint32_t threadIndex, size_t size);

    size_t size;
    bool transferToLocalDeviceMemory;

    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    static __CommandBuffer primaryCommandBuffer;
    static std::vector<std::vector<std::pair<__CommandBuffer, bool>>> stagingCommandBuffers;
    static std::vector<std::pair<VkBuffer, VkDeviceMemory>> stagingBufferAndMemoryDeleteQueue;
    static std::set<uint32_t> resetPoolIndexes;
    static __Fence finishedCopyingFence;

    std::shared_ptr<uint32_t> useCount;
};

}