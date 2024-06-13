#pragma once

#include <stdexcept>
#include <thread>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

typedef struct CommandPoolSet{
    VkCommandPool transferCommandPool;
    VkCommandPool graphicsCommandPool;
    uint32_t commandBufferCount = 1;
}CommandPoolSet;

class CommandPool{
public:
    static void CreateCommandPools(std::thread::id threadID);

    static VkCommandPool GetGraphicsCommandPool(std::thread::id threadID);
    static VkCommandPool GetTransferCommandPool(std::thread::id threadID);

    static void NotifyCommandBufferDestruction(std::thread::id threadID);

private:
    static std::unordered_map<std::thread::id, CommandPoolSet> commandPools;
};

}