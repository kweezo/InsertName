#pragma once

//IMPORTANT: Draw calls will be categorized by the pipeline they use

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "CommandPool.hpp"
#include "GraphicsPipeline.hpp"

#define COMMAND_BUFFER_GRAPHICS_FLAG 1
#define COMMAND_BUFFER_TRANSFER_FLAG 2

class CommandBuffer{
public:
    CommandBuffer(VkCommandBufferLevel level, uint32_t flags, GraphicsPipeline& pipeline);
    ~CommandBuffer();

    CommandBuffer(const CommandBuffer& other);
    CommandBuffer operator=(const CommandBuffer& other);

    void BeginCommandBuffer(uint32_t imageIndex);
    void EndCommandBuffer();

    VkCommandBuffer GetCommandBuffer();
private:

    uint32_t* useCount;

    GraphicsPipeline& pipeline;
    VkCommandBuffer commandBuffer;
};
