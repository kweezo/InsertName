#pragma once

//IMPORTANT: Draw calls will be categorized by the pipeline they use

#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "CommandPool.hpp"
#include "GraphicsPipeline.hpp"

#define COMMAND_BUFFER_GRAPHICS_FLAG 1
#define COMMAND_BUFFER_TRANSFER_FLAG 2

namespace renderer{

class CommandBuffer{
public:
    //You can ONLY pass the pipeline as nullptr if its a transfer command buffer
    CommandBuffer(VkCommandBufferLevel level, uint32_t flags, GraphicsPipeline* pipeline);
    CommandBuffer();
    ~CommandBuffer();

    CommandBuffer(const CommandBuffer& other);
    CommandBuffer operator=(const CommandBuffer& other);

    void BeginCommandBuffer(uint32_t imageIndex, VkCommandBufferInheritanceInfo* inheritanceInfo);
    void EndCommandBuffer();

    VkCommandBuffer GetCommandBuffer();
private:

    VkCommandBufferLevel level;
    uint32_t flags;

    uint32_t* useCount;

    GraphicsPipeline* pipeline;
    VkCommandBuffer commandBuffer;
};

}