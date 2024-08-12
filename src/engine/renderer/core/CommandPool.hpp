#pragma once

#include <thread>
#include <exception>

#include <vulkan/vulkan.h>

#include "Device.hpp"

#define COMMAND_POOL_TYPE_GRAPHICS 1
#define COMMAND_POOL_TYPE_TRANSFER 2

namespace renderer {
    typedef struct i_CommandPoolSet {
        VkCommandPool transferCommandPool;
        VkCommandPool graphicsCommandPool;
    } i_CommandPoolSet;

    enum i_CommandBufferType {
        // necessary evil :(
        GENERIC = 0,
        IMAGE = 1,
        DATA = 2,
        UNIFORM = 3,
        INSTANCE = 4,
        MODEL = 6,
        size = 6
    };


    class i_CommandPool {
    public:
        static void Init();

        static void Cleanup();


        static VkCommandPool GetTransferCommandPool(uint32_t poolID);

        static VkCommandPool GetGraphicsCommandPool(uint32_t poolID);

        static VkCommandPool ResetPool(uint32_t poolID); // TODO implement lol
    private:
        static std::vector<i_CommandPoolSet> commandPools;

        static void CreateCommandPool(uint32_t poolID);
    };
}
