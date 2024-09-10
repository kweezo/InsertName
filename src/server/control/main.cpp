#include "ServiceLink.hpp"
#include "AdvancedSettingsManager.hpp"
#include "AdminConsole.hpp"
#include "defines.hpp"

#include <thread>
#include <iostream>


int main() {
    AdvancedSettingsManager::LoadSettings(DIR + "control/config.json");
    AdminConsole::Init();
    Log::Init();

    std::thread connectionThread(&ServiceLink::StartTcpServer, AdvancedSettingsManager::GetSettings().controlServicePort);
    std::thread messageProcessingThread(&ServiceLink::ProcessMessages);
    std::thread messageSendingThread(&ServiceLink::ProcessSendBuffer);

    std::thread adminConsoleThread([]() {
        std::string line;
        while (AdminConsole::isRunning) {
            line = AdminConsole::ReadLine();
            AdminConsole::ProcessLine(line);
        }
    });


    // ----------------- Add here some other code ----------------- //


    messageSendingThread.join();
    connectionThread.join();
    messageProcessingThread.join();
    adminConsoleThread.join();

    endwin(); // Close ncurses
    #ifdef DEBUG
        std::cerr << "Exiting main" << std::endl;
    #endif
    return 0;
}
