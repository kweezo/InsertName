#include "ServiceLink.hpp"
#include "AdvancedSettingsManager.hpp"
#include "AdminConsole.hpp"

#include <thread>


int main() {
    AdvancedSettingsManager::LoadSettings("server_data/control/config.json");
    AdminConsole::Init();
    Log::Init();

    std::thread connectionThread(&ServiceLink::StartTcpServer, AdvancedSettingsManager::GetSettings().controlServicePort);
    std::thread messageProcessingThread(&ServiceLink::ProcessMessages);
    std::thread messageSendingThread(&ServiceLink::ProcessSendBuffer);

    std::thread adminConsoleThread([]() {
        std::string line;
        while (AdminConsole::IsRunning()) {
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
    return 0;
}
