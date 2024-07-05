#pragma once

#include <stdexcept>
#include <iostream>
#include <limits>
#include <memory>

#include <vulkan/vulkan.h>

#include "../window/Window.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"

namespace renderer{

struct __GraphicsPipelineCreateInfo{
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfo;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
};

class __GraphicsPipeline{
public:
    static void Init();
    static void Cleanup();

    __GraphicsPipeline();
    __GraphicsPipeline(__GraphicsPipelineCreateInfo createInfo);
    
    ~__GraphicsPipeline();

    __GraphicsPipeline(const __GraphicsPipeline& other);
    __GraphicsPipeline operator=(const __GraphicsPipeline& other);

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