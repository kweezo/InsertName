#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "network/NetworkClient.hpp"
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

// void chatThread(UserManager* userManager) {
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     std::string typeOfRequest;
// { // TEMPORARY CODE
//     struct TestStruct {
//         glm::vec3 v1[8];
//         glm::vec3 v2[8];
//     };

//     TestStruct ts;

//     // Set the values for v1
//     for (int i = 0; i < 8; ++i) {
//         ts.v1[i] = glm::vec3(i, i, i);
//     }

//     // Set the values for v2
//     for (int i = 0; i < 8; ++i) {
//         ts.v2[i] = glm::vec3(i * 2, i * 2, i * 2);
//     }
//     userManager->sendStruct(ts.v1, ts.v2);
// }
//     while (true) {
//         std::cout << "Enter type of request: ";
//         std::cin >> typeOfRequest;

//         if (typeOfRequest == "send") {
//             std::string receiver;
//             std::string message;
//             std::cout << "Enter receiver: ";
//             std::cin >> receiver;
//             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the newline left in the buffer by std::cin
//             std::cout << "Enter message: ";
//             std::getline(std::cin, message);
//             std::cout << userManager->sendMessage(receiver, message) << std::endl;
            
//         } else if (typeOfRequest == "get") {
//             std::string sender;
//             std::cout << "Enter sender: ";
//             std::cin >> sender;
//             std::cout << userManager->getChat(sender) << std::endl;

//         } else if (typeOfRequest == "getnew") {
//             std::string sender;
//             std::cout << "Enter sender: ";
//             std::cin >> sender;
//             std::cout << userManager->getNewMessages(sender) << std::endl;

//         } else if (typeOfRequest == "stop") {
//             std::cout << "Ending server connection" << std::endl;
//             userManager->closeConnection();
//             break;

//         } else {
//             std::cout << "Invalid request" << std::endl;
//         }
//         std::cout << std::endl;
//     }
// }

void networkTemp(){
    std::string server = "example.com";
    unsigned short port = 443;

    NetworkClient client(server, port);

    try {
        client.Start();

        std::this_thread::sleep_for(std::chrono::seconds(2));
        client.SendMessage("Hello, server!");

        std::this_thread::sleep_for(std::chrono::seconds(5));

        client.Stop();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
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
    networkTemp();

    Window::CreateWindowContext(settings.windowWidth, settings.windowHeight, "Vulkan");
    Renderer::Init();


    std::shared_ptr<__Shader> modelShader = __ShaderManager::GetShader("basicMesh");

    __ModelCreateInfo modelCreateInfo{};
    modelCreateInfo.path = dir + "res/models/teapot/teapot.fbx";
    //modelCreateInfo.shader


    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();

        int width;
        glfwGetWindowSize(Window::GetGLFWwindow(), &width, nullptr);
        if(!width){
            continue;
        }

        Renderer::Update();
    }
    vkDeviceWaitIdle(__Device::GetDevice());


    Renderer::Cleanup();
    Window::DestroyWindowContext();

    return 0;
}
