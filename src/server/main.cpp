#include "Server.hpp"
#include "Config.hpp"
#include "AdminConsole.hpp"
#include <thread>

int main() {
    std::string dir = "./server_data/";

    Config::GetInstance().LoadConfig(dir + "config.cfg");
    Server server(Config::GetInstance().serverPort, dir);
    server.initNetwork();

    // Create AdminConsole in a new thread
    std::thread adminConsoleThread([]() {
        AdminConsole adminConsole;
        std::string line;
        // Run the adminConsole in a loop
        while ((line = adminConsole.readLine("> ")) != "") {
            adminConsole.processLine(line);
        }
    });

    server.handleClients();

    // Join the adminConsoleThread before exiting the main function
    adminConsoleThread.join();

    return 0;
}