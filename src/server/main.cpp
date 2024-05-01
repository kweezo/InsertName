#include "Server.hpp"
#include <thread>

int main() {
    std::string dir = "./server_data/";

    Config::GetInstance().LoadConfig(dir + "config.cfg");
    Server server(12345, dir);
    server.initNetwork();

    while (true) {
        int clientSocket = server.acceptClient();
        std::thread clientThread(&Server::handleClient, &server, clientSocket);
        clientThread.detach();
    }

    return 0;
}