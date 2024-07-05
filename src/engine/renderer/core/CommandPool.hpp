#pragma once

#include <stdexcept>
#include <thread>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include "Device.hpp"

#define COMMAND_POOL_TYPE_GRAPHICS 0
#define COMMAND_POOL_TYPE_TRANSFER 1

namespace renderer{

typedef struct __CommandPoolSet{
    VkCommandPool transferCommandPool;
    VkCommandPool graphicsCommandPool;
    uint32_t commandBufferCount = 0;
}__CommandPoolSet;

class __CommandPool{
public:
    static void Cleanup();

    static void CreateCommandPools(uint32_t poolID);

    static void FreeCommandBuffer(VkCommandBuffer commandBuffer, uint32_t poolID, uint32_t commandPoolType);
    static VkCommandPool GetTransferCommandPool(uint32_t poolID);
    static VkCommandPool GetGraphicsCommandPool(uint32_t poolID);

    static VkCommandPool ResetPool(uint32_t poolID);// TODO implement lol
private:
    static std::unordered_map<uint32_t, __CommandPoolSet> commandPools;
};

}