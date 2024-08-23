#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "instance.hpp"

namespace renderer{

    class i_PhysicalDevice {
        public:
            static void Create();
            static void Destroy();

            static VkPhysicalDevice GetDevice();

            ~i_PhysicalDevice();
        private:
            i_PhysicalDevice();
            i_PhysicalDevice(const i_PhysicalDevice& other) = delete;
            i_PhysicalDevice& operator=(const i_PhysicalDevice& other) = delete; 


            static std::unique_ptr<i_PhysicalDevice> device;

            VkPhysicalDevice physicalDevice;


            void PickDevice();
    };

}