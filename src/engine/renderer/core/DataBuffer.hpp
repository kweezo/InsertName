#pragma once

#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <utility>
#include <thread>
#include <limits>
#include <set>
#include <list>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "Semaphore.hpp"


namespace renderer{

struct __DataBufferCreateInfo{
    void* data;
    size_t size;

    VkBufferUsageFlags usage;
    bool isDynamic;
    bool transferToLocalDeviceMemory;
    uint32_t threadIndex;

    __Semaphore signalSemaphore;
};

struct __DataBufferStagingCommandBuferData{
    __CommandBuffer commandBuffer;
    bool free;
    __Semaphore signalSemaphore;
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

    void SetSignalSemaphore(__Semaphore signalSemaphore);

    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void UploadDataToMemory(VkDeviceMemory memory, void* data, size_t size);
private:

    static void CreateCommandBuffers();
    static __CommandBuffer RetrieveFreeStagingCommandBuffer(uint32_t threadIndex, __Semaphore signalSemaphore);

    static void RecordPrimaryCommandBuffer();
    static void SubmitCommandBuffers();
    static void UpdateCleanup();

    void RecordCopyCommandBuffer(uint32_t threadIndex, size_t size);

    __DataBufferCreateInfo createInfo;

    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    static std::vector<std::vector<__DataBufferStagingCommandBuferData>> stagingCommandBuffers;
    static std::list<VkDeviceMemory> stagingMemoryDeleteQueue;
    static std::set<uint32_t> resetPoolIndexes;
    static __Fence finishedCopyingFence;
    static bool anyCommandBuffersRecorded;

    std::shared_ptr<uint32_t> useCount;
};

}