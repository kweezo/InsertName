#pragma once

//IMPORTANT: Draw calls will be categorized by the pipeline they use

#include <memory>
#include <mutex>
#include <deque>

#include <vulkan/vulkan.h>

#include "CommandPool.hpp"

#define COMMAND_BUFFER_GRAPHICS_FLAG 1
#define COMMAND_BUFFER_TRANSFER_FLAG 2
#define COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG 4

namespace renderer {
    struct i_CommandBufferCreateInfo {
        VkCommandBufferLevel level;
        uint32_t flags;
        i_CommandBufferType type;
        uint32_t threadIndex;
    };

    class i_CommandBuffer {
    public:
        //You can ONLY pass the pipeline as nullptr if its a transfer command buffer

        static void Init();

        i_CommandBuffer(i_CommandBufferCreateInfo createInfo);

        i_CommandBuffer();

        ~i_CommandBuffer();

        i_CommandBuffer(const i_CommandBuffer &other);

        i_CommandBuffer &operator=(const i_CommandBuffer &other);

        void ResetCommandBuffer() const;

        static void ResetPools(i_CommandBufferType type, uint32_t threadIndex);

        void BeginCommandBuffer(VkCommandBufferInheritanceInfo *inheritanceInfo, bool reset);

        void EndCommandBuffer();

        [[nodiscard]] VkCommandBuffer GetCommandBuffer() const;

        [[nodiscard]] uint32_t GetThreadIndex() const;

    private:
        void Destruct();

        VkCommandBufferLevel level;
        uint32_t flags;
        uint32_t poolID;

        uint32_t threadIndex;

        static std::deque<std::mutex> poolMutexes;
        std::unique_ptr<std::lock_guard<std::mutex> > lock;

        std::shared_ptr<uint32_t> useCount;

        VkCommandBuffer commandBuffer;
    };
}
