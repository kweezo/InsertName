#include "account/UserManager.hpp"
#include "../settings.hpp"
#include "renderer/window/Window.hpp"
#include "renderer/core/Renderer.hpp"
#include "renderer/core/CommandBuffer.hpp"
#include "renderer/core/Swapchain.hpp"
#include "renderer/core/Shader.hpp"
#include "renderer/core/GraphicsPipeline.hpp"
#include "renderer/core/VertexBuffer.hpp"

//implement staging and index buffer support (I am going to kill myself)

void userTemp(){
    UserManager userManager("127.0.0.1", 12345);
    if (userManager.connectToServer()) {
        userManager.loginUser('r', "username", "password");
    }
}

int main(){
    Settings settings;
    ReadSettings(settings, "src/settings.bin");
    //userTemp();

    Window::CreateWindowContext(settings.width, settings.height, "Vulkan");
    Renderer::InitRenderer();
{
    Shader shader = Shader("shaders/bin/triangleVert.spv", "shaders/bin/triangleFrag.spv"); // TEMP REMOVE LATER

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = sizeof(float)  * 3;
    

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = 6 * sizeof(float);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    float vertices[] = {
        0.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f
    };

    VertexBuffer vertexBuffer = VertexBuffer(attributeDescriptions, {bindingDescription}, sizeof(vertices),
     vertices, true);

    BufferDescriptions buffDescription = vertexBuffer.GetDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = buffDescription.bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = buffDescription.bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = buffDescription.attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = buffDescription.attributeDescriptions.data();
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
    depthStencilInfo.minDepthBounds = 0.0f;
    depthStencilInfo.maxDepthBounds = 1.0f;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    VkSubpassDescription subpass{};
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = Swapchain::GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;        


    VkSubpassDependency subpassDep{};
    subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDep.dstSubpass = 0;
    subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.srcAccessMask = 0;
    subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDep;
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    GraphicsPipeline pipeline = GraphicsPipeline(vertexInputInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, multisampling,
     depthStencilInfo, colorBlending, renderPassInfo, shader);

    CommandBuffer buffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG, &pipeline);

    uint32_t imageIndex = 0;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;



    if(vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
       vkCreateSemaphore(Device::GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
       vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS){ //cleanup is for losers anyways (ill do it later maybe lol)
        throw std::runtime_error("Failed to create semaphores");
    }

 

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();

        vkWaitForFences(Device::GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(Device::GetDevice(), 1, &inFlightFence);

        vkAcquireNextImageKHR(Device::GetDevice(), Swapchain::GetSwapchain(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);


        VkDeviceSize offsets[] = {0};

        buffer.BeginCommandBuffer(imageIndex, nullptr);
        VkBuffer buff = vertexBuffer.GetBuffer();
        vkCmdBindVertexBuffers(buffer.GetCommandBuffer(), 0, 1, &buff, offsets);
        vkCmdDraw(buffer.GetCommandBuffer(), 3, 1, 0, 0);
        buffer.EndCommandBuffer();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer = buffer.GetCommandBuffer();
        submitInfo.pCommandBuffers = &commandBuffer;

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

        if(vkQueueSubmit(Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS){
            throw std::runtime_error("Failed to submit draw command buffer");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphore;


        VkSwapchainKHR swapchain = Swapchain::GetSwapchain();
        presentInfo.pSwapchains = &swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;

        vkQueuePresentKHR(Device::GetGraphicsQueue(), &presentInfo);
        

        Renderer::RenderFrame();

        imageIndex = (imageIndex + 1) % 2;

    }

    vkDeviceWaitIdle(Device::GetDevice());

    vkDestroySemaphore(Device::GetDevice(), renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(Device::GetDevice(), imageAvailableSemaphore, nullptr);
    vkDestroyFence(Device::GetDevice(), inFlightFence, nullptr);
}
    Renderer::DestroyRenderer();
    Window::DestroyWindowContext();

    return 0;
}
