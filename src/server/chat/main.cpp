#include "shared/ClientServiceLink.hpp"

#include <thread>


int main() {
    std::thread connectionThread(&ClientServiceLink::StartClient);

    connectionThread.join();

    ClientServiceLink::DisconnectFromTcpServer();

    return 0;
}
