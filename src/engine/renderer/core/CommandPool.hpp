#pragma once

#include <stdexcept>
#include <thread>

#include <vulkan/vulkan.h>

#include "Device.hpp"

#define COMMAND_POOL_TYPE_GRAPHICS 1
#define COMMAND_POOL_TYPE_TRANSFER 2

namespace renderer{

typedef struct __CommandPoolSet{
    VkCommandPool transferCommandPool;
    VkCommandPool graphicsCommandPool;
}__CommandPoolSet;

enum __CommandBufferType{ // necessary evil :(
    GENERIC = 0,
    IMAGE = 1,
    DATA = 2,
    UNIFORM = 3,
    INSTANCE = 4,
    MODEL = 6,
    size = 6
};


class __CommandPool{
public:
    static void Init();
    static void Cleanup();


    static VkCommandPool GetTransferCommandPool(uint32_t poolID);
    static VkCommandPool GetGraphicsCommandPool(uint32_t poolID);

    static VkCommandPool ResetPool(uint32_t poolID);// TODO implement lol
private:
    static std::vector<__CommandPoolSet> commandPools;

    static void CreateCommandPool(uint32_t poolID);
};

}