#pragma once

#include <exception>
#include <tuple>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include <boost/container/flat_map.hpp>

#include "physicalDevice.hpp"

namespace renderer{

    struct i_QueueBatch{
        std::list<VkQueue> queues;
        VkQueueFlags flags;

        uint32_t queueFamilyIndex;
    };

    class i_LogicalDevice{
        public:
            static void Create();
            static void Destroy();

            static VkDevice GetDevice();
            static VmaAllocator GetAllocator();
            static std::vector<uint32_t> GetQueueFamilyIndices(VkQueueFlags flags);

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
            
            std::list<uint32_t> CheckExtensionSupport(const std::vector<char*>& extensions);

            std::vector<i_QueueBatch> queueBatches;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::list<std::vector<float>> queuePriorities;

            std::vector<char*> extensions;
            std::vector<char*> supportedOptionalExtensions;

            VkDevice logicalDevice;
            VmaAllocator vmaAllocator;
    };
}