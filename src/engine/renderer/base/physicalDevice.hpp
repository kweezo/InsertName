#pragma once

#include <memory>
#include <exception>
#include <list>
#include <tuple>

#include <vulkan/vulkan.h>

#include "instance.hpp"

namespace renderer{

    enum QueueType{
        GRAPHICS = 0,
        COMPUTE = 1,
        TRANSFER = 2,
        size = 3
    };

    class i_PhysicalDevice {
        public:
            static void Create();
            static void Destroy();

            static bool DeviceMemoryAvailable();

            static VkPhysicalDevice GetDevice();

        private:
            i_PhysicalDevice();
            i_PhysicalDevice(const i_PhysicalDevice& other) = delete;
            i_PhysicalDevice& operator=(const i_PhysicalDevice& other) = delete; 


            static std::unique_ptr<i_PhysicalDevice> device;

            void PickDevice();

            VkPhysicalDevice physicalDevice;
            bool memFree;
    };

}