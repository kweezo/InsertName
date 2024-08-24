#include "shared/ClientServiceLink.hpp"
#include "ClientHandler.hpp"
#include "messageHandler.hpp"
#include "Settings.hpp"
#include "defines.hpp"

#include <thread>
#include <chrono>


int main() {
    ClientServiceLink::SetMessageHandler(handleMessageContent);
    std::thread connectionThread(&ClientServiceLink::StartClient, DIR + "auth");

    while (!allSettingsReceived()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ClientHandler::Init(settings.port);



    // ----------------- Add here some other code ----------------- //



    connectionThread.join();

    ClientServiceLink::DisconnectFromTcpServer();

    return 0;
}
