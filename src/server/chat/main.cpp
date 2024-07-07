#include "ClientServiceLink.hpp"

#include <thread>


int main() {
    ClientServiceLink::ConnectToTcpServer("127.0.0.1", 8080);

    std::thread connectionThread(&ClientServiceLink::HandleConnection);

    connectionThread.join();

    ClientServiceLink::DisconnectFromTcpServer();

    return 0;
}