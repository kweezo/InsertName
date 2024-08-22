#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "../engine/app.hpp"

// #include "account/UserManager.hpp"
#include "account/Settings.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    // userManager->closeConnection();
    // delete userManager;

    renderer::WindowCreateInfo windowInfo{};

    windowInfo.fullscreen = false;
    windowInfo.vsync = false;
    windowInfo.windowName = "test";
    windowInfo.width = 800;
    windowInfo.height = 600;

    renderer::AppCreateInfo appInfo{};
    appInfo.name = "app";
    appInfo.version = APP_VERSION(0, 0, 1);
    appInfo.windowCreateInfo = windowInfo;

    renderer::App::Create(appInfo);

    while(!renderer::App::ShouldQuit()){

    }


    return 0;
}
