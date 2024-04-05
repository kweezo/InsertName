#include "Server.hpp"
#include <thread>

int main() {
    Server server(12345, "./server");
    server.initNetwork();

    while (true) {
        int clientSocket = server.acceptClient();
        server.handleClient(clientSocket);
    }

    return 0;
}