#include "account/UserManager.hpp"
#include "../settings.hpp"
#include "renderer/window/Window.hpp"
#include "renderer/core/Renderer.hpp"
#include "renderer/core/CommandBuffer.hpp"
#include "renderer/core/Swapchain.hpp"
#include "renderer/core/Shader.hpp"
#include "renderer/core/GraphicsPipeline.hpp"
#include "renderer/core/DataBuffer.hpp"
#include "renderer/core/Fence.hpp"
#include "renderer/core/DescriptorManager.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace renderer;//here beacuse this is again, all temp and i cant be bothered to actually refactor this properly

//implement staging and index buffer support (I am going to kill myself)

typedef struct Transform{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
} Transform;

void userTemp(){
    UserManager userManager("127.0.0.1", 12345);
    if (userManager.connectToServer()) {
        std::string username;
        std::string password;
        char loginType;
        std::cin >> loginType >> username >> password;
        userManager.loginUser(loginType, username, password);
    }
}

int main(){
    Settings settings;
    ReadSettings(settings, "src/settings.bin");
   // userTemp();

    Window::CreateWindowContext(settings.width, settings.height, "Vulkan");
    Renderer::InitRenderer();
{
    Shader shader = Shader("shaders/bin/triangleVert.spv", "shaders/bin/triangleFrag.spv"); // TEMP REMOVE LATER

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = sizeof(float)* 2;
    

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = 5 * sizeof(float);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    float vertices[] = {
        -0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    DataBuffer vertexBuffer = DataBuffer({attributeDescriptions, {bindingDescription}}, sizeof(vertices),
     vertices, true, DATA_BUFFER_VERTEX_BIT);

    DataBuffer indexBuffer = DataBuffer({}, sizeof(indices), indices, true, DATA_BUFFER_INDEX_BIT);

    BufferDescriptions buffDescription = vertexBuffer.GetDescriptions();

    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layoutBinding;

    DescriptorManager::Initialize({layoutInfo});

    DescriptorHandle descriptorHandle = DescriptorManager::CreateDescriptors({{0, 1}})[0];

    Transform transform;
    transform.model = glm::mat4(1.0f);
    transform.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    transform.projection = glm::perspective(glm::radians(45.0f), (float)settings.width / (float)settings.height, 0.1f, 10.0f);

    DataBuffer uniformBuffer = DataBuffer({}, sizeof(Transform), &transform, true, DATA_BUFFER_UNIFORM_BIT);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Transform);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = DescriptorManager::GetDescriptorSet(descriptorHandle); 
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(Device::GetDevice(), 1, &descriptorWrite, 0, nullptr);

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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = DescriptorManager::GetLayouts()[0].size();
    pipelineLayoutInfo.pSetLayouts = DescriptorManager::GetLayouts()[0].data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    GraphicsPipeline pipeline = GraphicsPipeline(vertexInputInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, multisampling,
     depthStencilInfo, colorBlending, renderPassInfo, pipelineLayoutInfo, shader);

    CommandBuffer buffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG, &pipeline);

    uint32_t imageIndex = 0;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //all temp code, not gonna bother implementing the fence wrapper haha
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

        VkDescriptorSet descriptor = DescriptorManager::GetDescriptorSet(descriptorHandle); 

        buffer.BeginCommandBuffer(imageIndex, nullptr);
        VkBuffer buff = vertexBuffer.GetBuffer();
        vkCmdBindVertexBuffers(buffer.GetCommandBuffer(), 0, 1, &buff, offsets);
        vkCmdBindIndexBuffer(buffer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(buffer.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1,
         &descriptor, 0, nullptr);
        vkCmdDrawIndexed(buffer.GetCommandBuffer(), 6, 1, 0, 0, 0);
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

        imageIndex = (imageIndex + 1) % Swapchain::GetImageCount(); 
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
