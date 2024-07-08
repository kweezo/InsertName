#include "ServiceLink.hpp"

#include <thread>


int main() {
    std::thread connectionThread(&ServiceLink::StartTcpServer, 8080);
    std::thread messageProcessingThread(&ServiceLink::ProcessMessages);

    connectionThread.join();
    messageProcessingThread.join();

    return 0;
}