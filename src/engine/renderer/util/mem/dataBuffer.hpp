#pragma once

#include <iostream>
#include <exception>
#include <memory>

#include <vulkan/vulkan.h>

#include <enkiTS/TaskScheduler.h>

#include "../../base/scheduler.hpp"
#include "../../base/logicalDevice.hpp"

namespace renderer{
    struct i_DataBufferCreateInfo{
        void* data;
        size_t size;

        VkBufferUsageFlags usage;
        VmaMemoryUsage memUsage;
        VkQueueFlags flags;
    };

    enum i_MemoryPriority{
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
    };

    enum i_DataBufferState{
        INIT
    };
    
    class i_DataBuffer : enki::ITaskSet{
        public:
            i_DataBuffer();
            i_DataBuffer(i_DataBufferCreateInfo createInfo);
            i_DataBuffer(const i_DataBuffer& other);
            i_DataBuffer& operator=(const i_DataBuffer& other);
            ~i_DataBuffer();

            void ExecuteRange(enki::TaskSetPartition range, uint32_t threadNum) override;
            void Destruct();

        private:
            void Init();

            void CreateBuffer();
            void CreateStagingBuffer();

            void CopyFrom(const i_DataBuffer& other);

            void UploadDataToStagingMemory();

            i_DataBufferState state;
            i_DataBufferCreateInfo createInfo;

            VkBuffer buffer;
            VmaAllocation allocation;

            VkBuffer stagingBuffer;
            VmaAllocation stagingAllocation;

            std::shared_ptr<uint32_t> useCount;
    };
}
