#pragma once

#include <stdexcept>
#include <cstring>
#include <memory>
#include <utility>
#include <thread>
#include <limits>
#include <set>
#include <list>
#include <mutex>
#include <format>
#include <tuple>

#include <vulkan/vulkan.h>

#include "Device.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "Fence.hpp"
#include "Semaphore.hpp"


namespace renderer{

struct _DataBufferCreateInfo{
    void* data;
    size_t size;

    VkBufferUsageFlags usage;
    bool isDynamic;
    bool transferToLocalDeviceMemory;
    uint32_t threadIndex;

    _Semaphore signalSemaphore;
};

struct _DataBufferStagingCommandBuferData{
    _CommandBuffer commandBuffer;
    std::weak_ptr<std::mutex> mutex;
    bool free;
    _Semaphore signalSemaphore;
};

class _DataBuffer{
public:
    static void Init();
    static void Update();
    static void Cleanup();


    _DataBuffer();
    _DataBuffer(_DataBufferCreateInfo createInfo);

    _DataBuffer(const _DataBuffer& other);
    _DataBuffer& operator=(const _DataBuffer& other);    

    ~_DataBuffer();

    void UpdateData(void* data, size_t size, uint32_t threadIndex);

    VkBuffer GetBuffer();

    void SetSignalSemaphore(_Semaphore signalSemaphore);

    static void CreateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceSize size);
    static void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, size_t size, VkMemoryPropertyFlags properties);
    static void UploadDataToMemory(VkDeviceMemory memory, void* data, size_t size);
private:

    void Destruct();

    static void CreateCommandBuffers();
    static _CommandBuffer RetrieveFreeStagingCommandBuffer(uint32_t threadIndex, _Semaphore signalSemaphore, std::weak_ptr<std::mutex> mutex);

    static void RecordPrimaryCommandBuffer();
    static void SubmitCommandBuffers();
    static void UpdateCleanup();

    void RecordCopyCommandBuffer(uint32_t threadIndex, size_t size);

    _DataBufferCreateInfo createInfo;

    std::shared_ptr<std::mutex>/*a minescule amount of tomfoolery*/ bufferMutex;
    VkBuffer buffer;
    VkDeviceMemory memory;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    static std::list<std::list<_DataBufferStagingCommandBuferData>> stagingCommandBuffers;
    static std::list<std::pair<VkBuffer, VkDeviceMemory>> stagingBufferAndMemoryDeleteQueue;
    static std::set<uint32_t> resetPoolIndexes;
    static _Fence finishedCopyingFence;
    static bool anyCommandBuffersRecorded;

    std::shared_ptr<uint32_t> useCount;
};

}