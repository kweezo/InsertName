#pragma once

#include <stdexcept>
#include <cstring>
#include <memory>
#include <utility>
#include <thread>
#include <set>
#include <list>
#include <mutex>
#include <format>

#include <vulkan/vulkan.h>

#include "Fence.hpp"
#include "Semaphore.hpp"
#include "CommandBuffer.hpp"


namespace renderer {
    struct i_DataBufferCreateInfo {
        void *data;
        size_t size;

        VkBufferUsageFlags usage;
        bool isDynamic;
        bool transferToLocalDeviceMemory;
        uint32_t threadIndex;

        i_Semaphore signalSemaphore;
    };

    struct i_DataBufferStagingCommandBuferData {
        i_CommandBuffer commandBuffer;
        std::weak_ptr<std::mutex> mutex;
        bool free;
        i_Semaphore signalSemaphore;
    };

    class i_DataBuffer {
    public:
        static void Init();

        static void Update();

        static void Cleanup();


        i_DataBuffer();

        i_DataBuffer(i_DataBufferCreateInfo createInfo);

        i_DataBuffer(const i_DataBuffer &other);
        i_DataBuffer &operator=(const i_DataBuffer &other);


        ~i_DataBuffer();

        void UpdateData(void *data, size_t size, uint32_t threadIndex);

        VkBuffer GetBuffer();

        void SetSignalSemaphore(i_Semaphore signalSemaphore);

        void Destruct();

        static void CreateBuffer(VkBuffer &buffer, VkBufferUsageFlags usage, VkDeviceSize size);

        static void AllocateMemory(VkDeviceMemory &memory, VkBuffer buffer, size_t size,
                                   VkMemoryPropertyFlags properties);

        static void UploadDataToMemory(VkDeviceMemory memory, void *data, size_t size);

    private:
        static void CreateCommandBuffers();

        static i_CommandBuffer RetrieveFreeStagingCommandBuffer(uint32_t threadIndex, i_Semaphore signalSemaphore,
                                                                std::weak_ptr<std::mutex> mutex);

        static void RecordPrimaryCommandBuffer();

        static void SubmitCommandBuffers();

        static void UpdateCleanup();

        static void WaitForQueue(VkQueue queue, std::list<std::pair<uint32_t, uint32_t>> buffersInQueue, std::shared_ptr<bool> finished);

        void RecordCopyCommandBuffer(uint32_t threadIndex, size_t size);

        i_DataBufferCreateInfo createInfo;

        std::shared_ptr<std::mutex>/*a minescule amount of tomfoolery*/ bufferMutex;
        VkBuffer buffer;
        VkDeviceMemory memory;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        std::mutex bufferCreationMutex; //made to handle a specific race condition

        static std::vector<std::vector<i_DataBufferStagingCommandBuferData> > stagingCommandBuffers;
        static std::list<std::pair<VkBuffer, VkDeviceMemory> > stagingBufferAndMemoryDeleteQueue;
        static std::vector<std::pair<std::shared_ptr<std::thread>, std::shared_ptr<bool>>> queueWaitThreads;
        static std::set<uint32_t> resetPoolIndexes;
        static i_Fence finishedCopyingFence;
        static bool anyCommandBuffersRecorded;

        std::shared_ptr<uint32_t> useCount;
    };
}
