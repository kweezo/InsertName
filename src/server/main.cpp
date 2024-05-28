#include "Log.hpp"
#include "Server.hpp"
#include "Config.hpp"
#include "AdminConsole.hpp"
#include <thread>

int main() {
    std::string dir = "./server_data/";

    Config::GetInstance().LoadConfig(dir + "config.cfg");
    Log::init();
    AdminConsole::init();

    // Run AdminConsole in a new thread
    std::thread adminConsoleThread([]() {
        std::string line;
        // Run the adminConsole in a loop
        while (true) {
            line = AdminConsole::readLine("> ");
            if (!line.empty()) {
                AdminConsole::processLine(line);
            }
        }
    });

    Server server(Config::GetInstance().serverPort, dir);
    server.initNetwork();
    
    server.handleClients();

    // Join the adminConsoleThread before exiting the main function
    adminConsoleThread.join();

    return 0;
}