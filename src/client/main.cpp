#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// #include "account/UserManager.hpp"
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

// void userTemp(UserManager* userManager){
//     if (userManager->connectToServer()) {
//         for (int i = 0; i < 3; i++) {
//             std::string username;
//             std::string password;
//             char loginType;
//             std::cin >> loginType >> username >> password;
//             if (userManager->loginUser(loginType, username, password) <= 2) {
//                 std::cout << "Successfuly logged in" << std::endl;
//                 break;
//             } else {
//                 std::cout << "Failed to log in" << std::endl;
//             }
//         }
        
//         std::thread chat(chatThread, userManager);
//         chat.detach();
//     }
// }


int main(){
    std::string dir = "./client_data/";

    Settings& settings = Settings::GetInstance();
    settings.LoadSettings(dir + "settings.cfg");
    // UserManager* userManager = new UserManager(settings.serverIP, settings.serverPort);
    // userTemp(userManager);

    Window::CreateWindowContext(settings.windowWidth, settings.windowHeight, "Vulkan");
    Renderer::Init();


    ShaderHandle modelShader = i_ShaderManager::GetShader("basicMesh");

    i_ModelCreateInfo modelCreateInfo{};
    modelCreateInfo.path = dir + "res/models/teapot/teapot.fbx";
    modelCreateInfo.name = "teapot";

    ModelHandle teapot = ModelManager::Create(modelCreateInfo);


    ModelInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.model = teapot;
    instanceCreateInfo.isDynamic = false;
    instanceCreateInfo.shader = modelShader;
    instanceCreateInfo.transform = {
        glm::vec3(0.0f, 0.0f, -5.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
    };

    ModelInstanceHandle teapotInstance = ModelInstance::Create(instanceCreateInfo);

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();

        int width;
        glfwGetWindowSize(Window::GetGLFWwindow(), &width, nullptr);
        if(!width){
            continue;
        }


        teapotInstance->TranslatePosition({0.01f, 0.f, 0.f});

        Renderer::Update();
    }
    vkDeviceWaitIdle(i_Device::GetDevice());

    teapotInstance.reset();

    Renderer::Cleanup();
    Window::DestroyWindowContext();

    // userManager->closeConnection();
    // delete userManager;

    return 0;
}
