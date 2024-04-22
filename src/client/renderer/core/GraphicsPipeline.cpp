#include "GraphicsPipeline.hpp"

namespace renderer{

GraphicsPipeline::GraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInputInfo, VkPrimitiveTopology topology,
    VkPolygonMode polygonMode, VkPipelineMultisampleStateCreateInfo multisampling,
     VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
      VkPipelineColorBlendStateCreateInfo colorBlending, VkRenderPassCreateInfo renderPassInfo, VkPipelineLayoutCreateInfo pipelineLayoutInfo,
       Shader& shader){
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = polygonMode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    if(vkCreatePipelineLayout(Device::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout");
    }

    if(vkCreateRenderPass(Device::GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
        throw std::runtime_error("Failed to create render pass");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shader.GetShaderStageCreateInfo().data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // MAY NEED TO RESTRUCTURE FOR THIS TO BE DYNAMIC idk i dont actually know what im doing
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if(vkCreateGraphicsPipelines(Device::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS){
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    std::vector<VkImageView> swapchainImageViews = Swapchain::GetSwapchainImageViews();

    framebuffers.resize(swapchainImageViews.size());

    for(int i = 0; i < swapchainImageViews.size(); i++){
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &swapchainImageViews[i];
        framebufferInfo.width = Swapchain::GetExtent().width;
        framebufferInfo.height = Swapchain::GetExtent().height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(Device::GetDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create framebuffer");
        }
    }



    useCount = new uint32_t;
    useCount[0] = 1;
}

void GraphicsPipeline::BeginRenderPassAndBindPipeline(uint32_t imageIndex, VkCommandBuffer commandBuffer){
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = Swapchain::GetExtent();
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f}; // todo, make this dynamic
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    //I will also just specify the scissors and the viewport here for convenience

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = Swapchain::GetExtent().width;
    viewport.height = Swapchain::GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = Swapchain::GetExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void GraphicsPipeline::EndRenderPass(VkCommandBuffer commandBuffer){
    vkCmdEndRenderPass(commandBuffer);
}

VkPipelineLayout GraphicsPipeline::GetPipelineLayout(){
    return pipelineLayout;
}

GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline& other){
    pipeline = other.pipeline;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    useCount = other.useCount;
    framebuffers = other.framebuffers;
    useCount[0]++;
}

GraphicsPipeline GraphicsPipeline::operator=(const GraphicsPipeline& other){
    if(this == &other){
        return *this;
    }

    pipeline = other.pipeline;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    useCount = other.useCount;
    framebuffers = other.framebuffers;
    useCount[0]++;
    return *this;
}

GraphicsPipeline::~GraphicsPipeline(){
    if(useCount[0] == 1){
        for(const VkFramebuffer& framebuffer : framebuffers){
            vkDestroyFramebuffer(Device::GetDevice(), framebuffer, nullptr);
        }
    
        vkDestroyPipeline(Device::GetDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(Device::GetDevice(), pipelineLayout, nullptr);
        vkDestroyRenderPass(Device::GetDevice(), renderPass, nullptr);
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}

}