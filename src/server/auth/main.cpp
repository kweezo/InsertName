#include "shared/ClientServiceLink.hpp"
#include "ClientHandler.hpp"
#include "messageHandler.hpp"
#include "Settings.hpp"
#include "defines.hpp"

#include <thread>
#include <chrono>
#include <iostream>


int main() {
    ClientServiceLink::SetMessageHandler(handleMessageContent);
    std::thread connectionThread(&ClientServiceLink::StartClient, DIR + "auth");

    while (!allSettingsReceived()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "All settings received." << std::endl;

    ClientHandler::Init(settings.port);
    ClientHandler::Start();



    // ----------------- Add here some other code ----------------- //



    connectionThread.join();

    std::cerr << "Exiting main" << std::endl;
    return 0;
}
