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

    static void Init();
    static void Cleanup();

    GraphicsPipeline();
    GraphicsPipeline(ShaderImpl& shader, BufferDescriptions& bufferDescriptions);
    
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline& other);
    GraphicsPipeline operator=(const GraphicsPipeline& other);

    void BeginRenderPassAndBindPipeline(uint32_t imageIndex, VkCommandBuffer commandBuffer);
    void EndRenderPass(VkCommandBuffer commandBuffer);

    VkPipelineLayout GetPipelineLayout();
    VkRenderPass GetRenderPass();
    VkFramebuffer GetFramebuffer(uint32_t index);
private:
    static void CreateRenderPass();
    static void CreateFramebuffers();

    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    static VkRenderPass renderPass;

    static std::vector <VkFramebuffer> framebuffers;

    uint32_t *useCount;
};

}