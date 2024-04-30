#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "account/UserManager.hpp"
#include "account/Settings.hpp"
#include "engine/renderer/window/Window.hpp"
#include "engine/renderer/core/Renderer.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/Swapchain.hpp"
#include "engine/renderer/core/Shader.hpp"
#include "engine/renderer/core/GraphicsPipeline.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/renderer/core/Fence.hpp"
#include "engine/renderer/core/DescriptorManager.hpp"
#include "engine/renderer/core/UniformBuffer.hpp"
#include "engine/renderer/core/Texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace renderer;//here beacuse this is again, all temp and i cant be bothered to actually refactor this properly

//implement staging and index buffer support (I am going to kill myself)

void chatThread(UserManager* userManager) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string typeOfRequest;
    while (true) {
        std::cout << "Enter type of request: ";
        std::cin >> typeOfRequest;

        if (typeOfRequest == "send") {
            std::string receiver;
            std::string message;
            std::cout << "Enter receiver: ";
            std::cin >> receiver;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the newline left in the buffer by std::cin
            std::cout << "Enter message: ";
            std::getline(std::cin, message);
            std::cout << userManager->sendMessage(receiver, message) << std::endl;
            
        } else if (typeOfRequest == "get") {
            std::string sender;
            std::cout << "Enter sender: ";
            std::cin >> sender;
            std::cout << userManager->getChat(sender) << std::endl;

        } else if (typeOfRequest == "getnew") {
            std::string sender;
            std::cout << "Enter sender: ";
            std::cin >> sender;
            std::cout << userManager->getNewMessages(sender) << std::endl;

        } else if (typeOfRequest == "stop") {
            std::cout << "Ending server connection" << std::endl;
            userManager->closeConnection();
            break;

        } else {
            std::cout << "Invalid request" << std::endl;
        }
        std::cout << std::endl;
    }
}

void userTemp(UserManager* userManager){
    if (userManager->connectToServer()) {
        for (int i = 0; i < 3; i++) {
            std::string username;
            std::string password;
            char loginType;
            std::cin >> loginType >> username >> password;
            if (userManager->loginUser(loginType, username, password) <= 2) {
                std::cout << "Successfuly logged in" << std::endl;
                break;
            } else {
                std::cout << "Failed to log in" << std::endl;
            }
        }
        
        std::thread chat(chatThread, userManager);
        chat.detach();
    }
}

struct ModelDat{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

int main(){
    std::string dir = "./client_data/";

    Settings& settings = Settings::GetInstance();
    settings.LoadSettings(dir + "settings.cfg");
    UserManager* userManager = new UserManager(settings.serverIP, settings.serverPort);
    userTemp(userManager);

    Window::CreateWindowContext(settings.windowWidth, settings.windowHeight, "Vulkan");
    Renderer::InitRenderer();
{

    VkDescriptorSetLayoutBinding uniformBufferBinding{};
    uniformBufferBinding.binding = 0;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.pImmutableSamplers = nullptr;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    ShaderHandle shader = Shader::CreateShader("shaders/bin/triangleVert.spv", "shaders/bin/triangleFrag.spv", "triangle", {uniformBufferBinding, samplerBinding}); // TEMP REMOVE LATER
    Shader::EnableNewShaders();

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

    ModelDat modelDat;
    modelDat.model = glm::mat4(1.0f);
    modelDat.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelDat.proj = glm::perspective(glm::radians(45.0f), (float)Settings::GetInstance().windowWidth / Settings::GetInstance().windowHeight, 0.1f, 10.0f);
    UniformBufferHandle uniformBuffer = UniformBuffer::Create(reinterpret_cast<void*>(&modelDat), sizeof(modelDat), 0,
    shader->GetDescriptorSet()); 

    GraphicsPipeline pipeline = GraphicsPipeline(*shader, buffDescription);

    CommandBuffer buffer = CommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, COMMAND_BUFFER_GRAPHICS_FLAG);

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

    TextureHandle texture = Texture::CreateTexture(dir + "res/textures/test.jpeg", 1, shader->GetDescriptorSet());
    Texture::EnableTextures();


    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();

        int width;
        glfwGetWindowSize(Window::GetGLFWwindow(), &width, nullptr);
        if(!width){
            continue;
        }

        vkWaitForFences(Device::GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(Device::GetDevice(), 1, &inFlightFence);

        vkAcquireNextImageKHR(Device::GetDevice(), Swapchain::GetSwapchain(), UINT64_MAX, imageAvailableSemaphore,
         VK_NULL_HANDLE, &imageIndex);


        VkDeviceSize offsets[] = {0};

        modelDat.model = glm::rotate(glm::mat4(1.0f), (float)cos(glfwGetTime()), glm::vec3(0.0f, 0.0f, 1.0f));
        uniformBuffer->UpdateData(reinterpret_cast<void*>(&modelDat), sizeof(modelDat));

        std::vector<VkWriteDescriptorSet> descriptorWrite = {
            texture->GetWriteDescriptorSet(),
            uniformBuffer->GetWriteDescriptorSet(),
        };


        buffer.BeginCommandBuffer(imageIndex, nullptr);
        pipeline.BeginRenderPassAndBindPipeline(imageIndex, buffer.GetCommandBuffer());
        shader->UpdateDescriptorSet(descriptorWrite);
        shader->Bind(buffer.GetCommandBuffer(), pipeline.GetPipelineLayout());
        VkBuffer buff = vertexBuffer.GetBuffer();
        vkCmdBindVertexBuffers(buffer.GetCommandBuffer(), 0, 1, &buff, offsets);
        vkCmdBindIndexBuffer(buffer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(buffer.GetCommandBuffer(), 6, 1, 0, 0, 0);
        pipeline.EndRenderPass(buffer.GetCommandBuffer());
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

    UniformBuffer::Free(uniformBuffer);
    Texture::Free(texture);
    Shader::Free(shader);

    vkDestroySemaphore(Device::GetDevice(), renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(Device::GetDevice(), imageAvailableSemaphore, nullptr);
    vkDestroyFence(Device::GetDevice(), inFlightFence, nullptr);
    
}
    Renderer::DestroyRenderer();
    Window::DestroyWindowContext();

    delete userManager;

    return 0;
}
