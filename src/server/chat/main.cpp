#include "shared/ClientServiceLink.hpp"
#include "messageHandler.hpp"

#include <thread>


int main() {
    ClientServiceLink::SetMessageHandler(handleMessageContent);
    std::thread connectionThread(&ClientServiceLink::StartClient, "server_data/chat");

    connectionThread.join();

    ClientServiceLink::DisconnectFromTcpServer();

    return 0;
}
