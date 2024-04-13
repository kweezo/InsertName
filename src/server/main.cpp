#include "Server.hpp"
#include <thread>

int main() {
    Server server(12345, "./server_data");
    server.initNetwork();

    while (true) {
        int clientSocket = server.acceptClient();
        server.handleClient(clientSocket);
    }

    return 0;
}