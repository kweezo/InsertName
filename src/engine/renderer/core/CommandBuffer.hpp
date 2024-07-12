#pragma once

//IMPORTANT: Draw calls will be categorized by the pipeline they use

#include <stdexcept>
#include <memory>
#include <thread>
#include <array>
#include <thread>
#include <mutex>
#include <deque>

#include <vulkan/vulkan.h>

#include "CommandPool.hpp"

#define COMMAND_BUFFER_GRAPHICS_FLAG 1
#define COMMAND_BUFFER_TRANSFER_FLAG 2
#define COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG 4

namespace renderer{

enum __CommandBufferType{
    GENERIC = 0,
    IMAGE = 1,
    DATA = 2,
    UNIFORM = 3,
    INSTANCE = 4,
    MODEL = 6,
    size = 6
};

struct __CommandBufferCreateInfo{
    VkCommandBufferLevel level;
    uint32_t flags;
    __CommandBufferType type;
    uint32_t threadIndex;
};

class __CommandBuffer{
public:
    //You can ONLY pass the pipeline as nullptr if its a transfer command buffer

    static void Init();

    __CommandBuffer(__CommandBufferCreateInfo createInfo);
    __CommandBuffer();
    ~__CommandBuffer();

    __CommandBuffer(const __CommandBuffer& other);
    __CommandBuffer operator=(const __CommandBuffer& other);

    void ResetCommandBuffer();
    static void ResetPools(__CommandBufferType type, uint32_t threadIndex);

    void BeginCommandBuffer(VkCommandBufferInheritanceInfo* inheritanceInfo, bool reset);
    void EndCommandBuffer();

    VkCommandBuffer GetCommandBuffer();
private:

    VkCommandBufferLevel level;
    uint32_t flags;
    uint32_t poolID;

    static std::deque<std::mutex> poolMutexes;
    std::unique_ptr<std::lock_guard<std::mutex>> lock;

    std::shared_ptr<uint32_t> useCount;

    VkCommandBuffer commandBuffer;
};

}