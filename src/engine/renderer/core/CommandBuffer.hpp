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

struct _CommandBufferCreateInfo{
    VkCommandBufferLevel level;
    uint32_t flags;
    _CommandBufferType type;
    uint32_t threadIndex;
};

class _CommandBuffer{
public:
    //You can ONLY pass the pipeline as nullptr if its a transfer command buffer

    static void Init();

    _CommandBuffer(_CommandBufferCreateInfo createInfo);
    _CommandBuffer();
    ~_CommandBuffer();

    _CommandBuffer(const _CommandBuffer& other);
    _CommandBuffer operator=(const _CommandBuffer& other);

    void ResetCommandBuffer();
    static void ResetPools(_CommandBufferType type, uint32_t threadIndex);

    void BeginCommandBuffer(VkCommandBufferInheritanceInfo* inheritanceInfo, bool reset);
    void EndCommandBuffer();

    VkCommandBuffer GetCommandBuffer();
private:
    void Destruct();

    VkCommandBufferLevel level;
    uint32_t flags;
    uint32_t poolID;

    static std::deque<std::mutex> poolMutexes;
    std::unique_ptr<std::lock_guard<std::mutex>> lock;

    std::shared_ptr<uint32_t> useCount;

    VkCommandBuffer commandBuffer;
};

}