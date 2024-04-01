#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

class CommandPool{
public:
    static void CreateCommandPools();
    static void DestroyCommandPools();

    static VkCommandPool GetGraphicsCommandPool();
    static VkCommandPool GetTransferCommandPool();

private:
    static VkCommandPool graphicsCommandPool;
    static VkCommandPool transferCommandPool;
};

}