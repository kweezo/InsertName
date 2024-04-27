#pragma once

#include <stdexcept>
#include <iostream>
#include <limits>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
namespace renderer{

class GraphicsPipeline{
public:
    GraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInputInfo, VkPrimitiveTopology topology,
    VkPolygonMode polygonMode, VkPipelineMultisampleStateCreateInfo multisampling,
     VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
      VkPipelineColorBlendStateCreateInfo colorBlendAttachment,
       VkRenderPassCreateInfo renderPassInfo, VkPipelineLayoutCreateInfo pipelineLayoutInfo,
        ShaderImpl& shader);
    
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