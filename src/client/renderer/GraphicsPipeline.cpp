#include "GraphicsPipeline.hpp"

GraphicsPipeline::GraphicsPipeline(VkPipelineVertexInputStateCreateInfo vertexInputInfo, VkPrimitiveTopology topology,
    VkPolygonMode polygonMode, VkPipelineMultisampleStateCreateInfo multisampling,
     VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
      VkPipelineColorBlendStateCreateInfo colorBlending, VkRenderPassCreateInfo renderPassInfo,
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

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

    useCount = new uint32_t;
    useCount[0] = 1;
}

GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline& other){
    pipeline = other.pipeline;
    pipelineLayout = other.pipelineLayout;
    renderPass = other.renderPass;
    useCount = other.useCount;
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
    useCount[0]++;
    return *this;
}

GraphicsPipeline::~GraphicsPipeline(){
    if(useCount[0] == 1){
        vkDestroyPipeline(Device::GetDevice(), pipeline, nullptr);
        vkDestroyPipelineLayout(Device::GetDevice(), pipelineLayout, nullptr);
        vkDestroyRenderPass(Device::GetDevice(), renderPass, nullptr);
        delete[] useCount;
    }
    else{
        useCount[0]--;
    }
}