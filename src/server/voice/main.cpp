#include "shared/ClientServiceLink.hpp"

#include <thread>


int main() {
    std::thread connectionThread(&ClientServiceLink::StartClient, "server_data/voice");

    connectionThread.join();

    ClientServiceLink::DisconnectFromTcpServer();

    return 0;
}
