#include "Server.hpp"
#include "Config.hpp"
#include <thread>

int main() {
    std::string dir = "./server_data/";

    Config::GetInstance().LoadConfig(dir + "config.cfg");
    Server server(Config::GetInstance().serverPort, dir);
    server.initNetwork();

    while (true) {
        int clientSocket = server.acceptClient();
        std::thread clientThread([clientSocket, &server]() {
            std::unique_ptr<ClientHandler> handler = std::make_unique<ClientHandler>(clientSocket, server.ssl, *server.c);
            handler->handleConnection();
        });
        clientThread.detach();
    }

    return 0;
}
