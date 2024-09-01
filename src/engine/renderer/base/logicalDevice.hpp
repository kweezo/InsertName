#pragma once

#include <exception>
#include <tuple>
#include <memory>

#include <vulkan/vulkan.h>

#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include "physicalDevice.hpp"

namespace renderer{

    struct i_QueueBatch{
        std::list<VkQueue> queues;
        VkQueueFlags flags;
    };

    class i_LogicalDevice{
        public:
            static void Create();
            static void Destroy();

            static VkDevice GetDevice();
            static VmaAllocator GetAllocator();

            ~i_LogicalDevice();
    
        private:
            i_LogicalDevice();
            i_LogicalDevice(const i_LogicalDevice& other) = delete;
            i_LogicalDevice& operator=(const i_LogicalDevice& other) = delete;

            static std::unique_ptr<i_LogicalDevice> device;

            void LoadConfig();
            void CreateQueueCreateInfos();
            void CreateDevice();
            void RetrieveQueues();
            void CreateVulkanAllocator();

            std::vector<i_QueueBatch> queueBatches;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::list<std::vector<float>> queuePriorities;

            std::vector<char*> extensions;

            VkDevice logicalDevice;
            VmaAllocator vmaAllocator;

    };
}