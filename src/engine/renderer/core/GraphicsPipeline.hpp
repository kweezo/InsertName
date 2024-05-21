#pragma once

#include <stdexcept>
#include <iostream>
#include <limits>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "DataBuffer.hpp"

namespace renderer{

class GraphicsPipeline{
public:
    GraphicsPipeline();
    GraphicsPipeline(ShaderImpl& shader, BufferDescriptions& bufferDescriptions);
    
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline& other);
    GraphicsPipeline operator=(const GraphicsPipeline& other);

    void BeginRenderPassAndBindPipeline(uint32_t imageIndex, VkCommandBuffer commandBuffer);
    void EndRenderPass(VkCommandBuffer commandBuffer);

    VkPipelineLayout GetPipelineLayout();
    VkRenderPass GetRenderPass();
private:
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    std::vector <VkFramebuffer> framebuffers;

    uint32_t *useCount;
};

}