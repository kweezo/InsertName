#include "Server.hpp"
#include "Config.hpp"

int main() {
    std::string dir = "./server_data/";

    Config::GetInstance().LoadConfig(dir + "config.cfg");
    Server server(Config::GetInstance().serverPort, dir);
    server.initNetwork();
    server.handleClients();

    return 0;
}