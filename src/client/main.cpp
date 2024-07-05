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
#include "engine/renderer/ext/model/Model.hpp"
#include "engine/renderer/ext/model/ModelInstance.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace renderer;//here beacuse this is again, all temp and i cant be bothered to actually refactor this properly

//implement staging and index buffer support (I am going to kill myself)

void chatThread(UserManager* userManager) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string typeOfRequest;
{ // TEMPORARY CODE
    struct TestStruct {
        glm::vec3 v1[8];
        glm::vec3 v2[8];
    };

    TestStruct ts;

    // Set the values for v1
    for (int i = 0; i < 8; ++i) {
        ts.v1[i] = glm::vec3(i, i, i);
    }

    // Set the values for v2
    for (int i = 0; i < 8; ++i) {
        ts.v2[i] = glm::vec3(i * 2, i * 2, i * 2);
    }
    userManager->sendStruct(ts.v1, ts.v2);
}
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
    Renderer::Init();


    __Shader* shader = ShaderManager::GetShader("triangle");

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

    __DataBufferCreateInfo vCreateInfo{};
    vCreateInfo.data = vertices;
    vCreateInfo.size = sizeof(vertices);
    vCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vCreateInfo.transferToLocalDeviceMemory = true;

    __DataBufferCreateInfo iCreateInfo{};
    iCreateInfo.data = indices;
    iCreateInfo.size = sizeof(indices);
    iCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    iCreateInfo.transferToLocalDeviceMemory = true;

    __DataBuffer vertexBuffer(vCreateInfo);
    __DataBuffer indexBuffer(iCreateInfo);

    ModelDat modelDat{};
    modelDat.model = glm::mat4(1.0f);
    modelDat.view = glm::mat4(1.0f);
    modelDat.proj = glm::perspective(glm::radians(45.0f), (float)Window::GetExtent().width / (float)Window::GetExtent().height, 0.1f, 100.0f);

    shader->CreateGraphicsPipepeline({attributeDescriptions, {bindingDescription}});

    __CommandBufferCreateInfo createInfo{};
    createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    createInfo.flags = COMMAND_BUFFER_GRAPHICS_FLAG;
    createInfo.threadIndex = 0;
    createInfo.type = __CommandBufferType::GENERIC;

    __CommandBuffer commandBuffer(createInfo);

    __Semaphore imageAvailableSemaphore;
    __Semaphore renderFinishedSemaphore;
    __Fence inFlightFence;

    __TextureCreateInfo textureCreateInfo{};
    textureCreateInfo.path = dir + "res/textures/test.jpeg";
    textureCreateInfo.binding = 1;
    textureCreateInfo.descriptorSet = shader->GetDescriptorSet();

    __Texture texture = __Texture(textureCreateInfo);

    __Texture::Update();

    uint32_t imageIndex;

    __UniformBufferCreateInfo uCreateInfo{};
    uCreateInfo.data = &modelDat;
    uCreateInfo.size = sizeof(ModelDat);
    uCreateInfo.binding = 0;
    uCreateInfo.descriptorSet = shader->GetDescriptorSet();

    __UniformBuffer* uniformb = new __UniformBuffer(uCreateInfo);

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();

        int width;
        glfwGetWindowSize(Window::GetGLFWwindow(), &width, nullptr);
        if(!width){
            continue;
        }

        VkFence inFlightFenceHandle = inFlightFence.GetFence();
        VkSemaphore imageAvailableSemaphoreHandle = imageAvailableSemaphore.GetSemaphore();
        VkSemaphore renderFinishedSemaphoreHandle = renderFinishedSemaphore.GetSemaphore();
        vkWaitForFences(__Device::GetDevice(), 1, &inFlightFenceHandle, VK_TRUE, UINT64_MAX);
        vkResetFences(__Device::GetDevice(), 1, &inFlightFenceHandle);

        vkAcquireNextImageKHR(__Device::GetDevice(), __Swapchain::GetSwapchain(), UINT64_MAX, imageAvailableSemaphore.GetSemaphore(), VK_NULL_HANDLE, &imageIndex);

        VkDeviceSize offsets[] = {0};

        std::vector<VkWriteDescriptorSet> descriptorWrite = {
            texture.GetWriteDescriptorSet(),
            uniformb->GetWriteDescriptorSet()
        };

        commandBuffer.BeginCommandBuffer(nullptr, true);
        shader->GetGraphicsPipeline()->BeginRenderPassAndBindPipeline(imageIndex, commandBuffer.GetCommandBuffer());
        shader->UpdateDescriptorSet(descriptorWrite);

        VkBuffer vBuffer = vertexBuffer.GetBuffer();

        vkCmdBindVertexBuffers(commandBuffer.GetCommandBuffer(), 0, 1, &vBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer.GetCommandBuffer(), 6, 1, 0, 0, 0);
        shader->GetGraphicsPipeline()->EndRenderPass(commandBuffer.GetCommandBuffer());
        commandBuffer.EndCommandBuffer();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphoreHandle};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer commandBuffers[] = {commandBuffer.GetCommandBuffer()};

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffers;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphoreHandle};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if(vkQueueSubmit(__Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFenceHandle) != VK_SUCCESS){
            throw std::runtime_error("Failed to submit draw command buffer");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;

        vkQueuePresentKHR(__Device::GetGraphicsQueue(), &presentInfo);

        Renderer::RenderFrame();
    }

    Renderer::Cleanup();
    Window::DestroyWindowContext();

    userManager->closeConnection();
    delete userManager;

    return 0;
}
