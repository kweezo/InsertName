#pragma once

#include <stdexcept>
#include <iostream>
#include <limits>
#include <memory>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "DataBuffer.hpp"

namespace renderer{

struct GraphicsPipelineCreateInfo{
    Shader* shader;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
};

class GraphicsPipeline{
public:
    static void Init();
    static void Cleanup();

    GraphicsPipeline();
    GraphicsPipeline(GraphicsPipelineCreateInfo createInfo);
    
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

    static std::vector<VkFramebuffer> framebuffers;

    std::shared_ptr<uint32_t> useCount;
};

}