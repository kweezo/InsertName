#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// #include "engine/app.hpp"

#include "network/NetworkClient.hpp"
// #include "account/Settings.hpp"


// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

//implement staging and index buffer support (I am going to kill myself)

// void chatThread(UserManager* userManager) {
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     std::string typeOfRequest;
// { // TEMPORARY CODE
//     struct TestStruct {
//         glm::vec3 v1[8];
//         glm::vec3 v2[8];
//     };
// void chatThread(UserManager* userManager) {
//     std::this_thread::sleep_for(std::chrono::seconds(1));
//     std::string typeOfRequest;
// { // TEMPORARY CODE
//     struct TestStruct {
//         glm::vec3 v1[8];
//         glm::vec3 v2[8];
//     };

//     TestStruct ts;
//     TestStruct ts;

//     // Set the values for v1
//     for (int i = 0; i < 8; ++i) {
//         ts.v1[i] = glm::vec3(i, i, i);
//     }
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
//         } else if (typeOfRequest == "getnew") {
//             std::string sender;
//             std::cout << "Enter sender: ";
//             std::cin >> sender;
//             std::cout << userManager->getNewMessages(sender) << std::endl;

//         } else if (typeOfRequest == "stop") {
//             std::cout << "Ending server connection" << std::endl;
//             userManager->closeConnection();
//             break;
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
//         } else {
//             std::cout << "Invalid request" << std::endl;
//         }
//         std::cout << std::endl;
//     }
// }

void networkTemp(){
    std::string server = "127.0.0.1";
    unsigned short port = 8083;

    NetworkClient client(server, port);

    client.Start();

    client.SendMessage("Hello, server!");

    client.Stop();
}

// struct ModelDat{
//     glm::mat4 model;
//     glm::mat4 view;
//     glm::mat4 proj;
// };

int main(){
    // std::string dir = "./client_data/";

    // Settings& settings = Settings::GetInstance();
    // settings.LoadSettings(dir + "settings.cfg");
    
    networkTemp();

    // renderer::WindowCreateInfo windowInfo{};

    // windowInfo.fullscreen = false;
    // windowInfo.vsync = false;
    // windowInfo.windowName = "test";
    // windowInfo.width = 800;
    // windowInfo.height = 600;

    // renderer::AppCreateInfo appInfo{};
    // appInfo.name = "app";
    // appInfo.version = APP_VERSION(0, 0, 1);
    // appInfo.windowCreateInfo = windowInfo;

    // renderer::App::Create(appInfo);

    // while(!renderer::App::ShouldQuit()){
    //     renderer::App::Update();
    // }

    // networkTemp();

    // Window::CreateWindowContext(settings.windowWidth, settings.windowHeight, "Vulkan");
    // Renderer::Init();


    // std::weak_ptr<_Shader> modelShader = _ShaderManager::GetShader("basicMesh");

    // _ModelCreateInfo modelCreateInfo{};
    // modelCreateInfo.path = dir + "res/models/teapot/teapot.fbx";
    // modelCreateInfo.shader = _ShaderManager::GetShader("basicMesh");
    // modelCreateInfo.name = "teapot";

    // ModelHandle teapot = ModelManager::Create(modelCreateInfo);


    // ModelInstanceCreateInfo instanceCreateInfo{};
    // instanceCreateInfo.model = teapot;
    // instanceCreateInfo.isDynamic = false;
    // instanceCreateInfo.transform = {
    //     glm::vec3(0.0f, 0.0f, -0.0f),
    //     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    //     glm::vec3(1.0f, 1.0f, 1.0f)
    // };

    // ModelInstanceHandle teapotInstance = ModelInstance::Create(instanceCreateInfo);

    // while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
    //     glfwPollEvents();

    //     int width;
    //     glfwGetWindowSize(Window::GetGLFWwindow(), &width, nullptr);
    //     if(!width){
    //         continue;
    //     }

    //     Renderer::Update();
    // }
    // vkDeviceWaitIdle(_Device::GetDevice());


    // Renderer::Cleanup();
    // Window::DestroyWindowContext();

    return 0;
}
