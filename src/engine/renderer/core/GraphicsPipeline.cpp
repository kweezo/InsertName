#include "GraphicsPipeline.hpp"

namespace renderer{

VkRenderPass _GraphicsPipeline::renderPass = {};
std::vector <VkFramebuffer> _GraphicsPipeline::framebuffers = {};

_GraphicsPipeline::_GraphicsPipeline(): pipeline(VK_NULL_HANDLE){
}

void _GraphicsPipeline::Init(){
    CreateRenderPass();
    CreateFramebuffers();
}
void _GraphicsPipeline::CreateRenderPass(){
    VkSubpassDescription subpass{};

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;



    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _Swapchain::GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;        

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = _Swapchain::GetDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    VkSubpassDependency subpassDep{};
    subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDep.dstSubpass = 0;
    subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDep.srcAccessMask = 0;
    subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDep;

    if(vkCreateRenderPass(_Device::GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
        throw std::runtime_error("Failed to create render pass");
    }
}
void _GraphicsPipeline::CreateFramebuffers(){
    std::vector<VkImageView> swapchainImageViews = _Swapchain::GetSwapchainImageViews();

    framebuffers.resize(swapchainImageViews.size());

    for(int i = 0; i < swapchainImageViews.size(); i++){

        std::vector<VkImageView> attachments = {swapchainImageViews[i], _Swapchain::GetDepthImage().GetImageView()};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _Swapchain::GetExtent().width;
        framebufferInfo.height = _Swapchain::GetExtent().height;
        framebufferInfo.layers = 1;


        if(framebufferInfo.width == std::numeric_limits<uint32_t>::max() && framebufferInfo.height == std::numeric_limits<uint32_t>::max()){
            int width, height;
            glfwGetWindowSize(Window::GetGLFWwindow(), &width, &height);
            framebufferInfo.width = width;
            framebufferInfo.height = height;
        }

        if(vkCreateFramebuffer(_Device::GetDevice(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

void _GraphicsPipeline::Cleanup(){
    vkDestroyRenderPass(_Device::GetDevice(), renderPass, nullptr);
    for(VkFramebuffer framebuffer : framebuffers){
        vkDestroyFramebuffer(_Device::GetDevice(), framebuffer, nullptr);
    }
}

_GraphicsPipeline::_GraphicsPipeline(_GraphicsPipelineCreateInfo createInfo){
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    //its a clusterfuck that HAS to exist, may god have mercy on us all

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = createInfo.bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = createInfo.bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = createInfo.attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = createInfo.attributeDescriptions.data();
 

    std::vector<VkDescriptorSetLayout> layouts = _DescriptorManager::GetLayouts();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;


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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    if(vkCreatePipelineLayout(_Device::GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout");
    }



    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = createInfo.shaderStageCreateInfo.data();
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

    if(vkCreateGraphicsPipelines(_Device::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS){
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    useCount = std::make_shared<uint32_t>(1);
}

void _GraphicsPipeline::BeginRenderPassAndBindPipeline(uint32_t imageIndex, VkCommandBuffer commandBuffer){
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _Swapchain::GetExtent();
    if(renderPassInfo.renderArea.extent.width == -1 || renderPassInfo.renderArea.extent.height == -1){
        glfwGetWindowSize(Window::GetGLFWwindow(), (int*)&renderPassInfo.renderArea.extent.width, (int*)&renderPassInfo.renderArea.extent.height);
    }

    std::vector<VkClearValue> clearValues = {VkClearValue{1.0f, 0.0f, 0.0f, 1.0f}, VkClearValue{1.0f, 0}};

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    //I will also just specify the scissors and the viewport here for convenience

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = renderPassInfo.renderArea.extent.width;
    viewport.height = renderPassInfo.renderArea.extent.height;

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderPassInfo.renderArea.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void _GraphicsPipeline::EndRenderPass(VkCommandBuffer commandBuffer){
    vkCmdEndRenderPass(commandBuffer);
}
VkPipelineLayout _GraphicsPipeline::GetPipelineLayout(){
    return pipelineLayout;
}
    
VkFramebuffer _GraphicsPipeline::GetFramebuffer(uint32_t index){
    return framebuffers[index];
}

VkRenderPass _GraphicsPipeline::GetRenderPass(){
    return renderPass;
}

_GraphicsPipeline::_GraphicsPipeline(const _GraphicsPipeline& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    pipeline = other.pipeline;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    useCount = other.useCount;
    framebuffers = other.framebuffers;
    (*useCount.get())++;
}

_GraphicsPipeline _GraphicsPipeline::operator=(const _GraphicsPipeline& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    pipeline = other.pipeline;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    useCount = other.useCount;
    framebuffers = other.framebuffers;
    (*useCount.get())++;

    return *this;
}

void _GraphicsPipeline::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount == 1){
        if(pipeline != VK_NULL_HANDLE){
            vkDestroyPipelineLayout(_Device::GetDevice(), pipelineLayout, nullptr);
            vkDestroyPipeline(_Device::GetDevice(), pipeline, nullptr);
        }
        useCount.reset();
    }
    else{
        (*useCount.get())--;
    }
}

_GraphicsPipeline::~_GraphicsPipeline(){
    Destruct();
}

}