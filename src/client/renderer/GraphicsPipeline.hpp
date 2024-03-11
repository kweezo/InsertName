#pragma once

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Shader.hpp"

class GraphicsPipeline{
public:
    GraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInputInfo, VkPrimitiveTopology topology,
    VkPolygonMode polygonMode, VkPipelineMultisampleStateCreateInfo multisampling,
     VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
      VkPipelineColorBlendStateCreateInfo colorBlendAttachment,
       VkRenderPassCreateInfo renderPassInfo,
        Shader& shader);
    
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline& other);
    GraphicsPipeline operator=(const GraphicsPipeline& other);

    VkRenderPass GetRenderPass();
private:
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    uint32_t *useCount;
};