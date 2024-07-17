#include "ServiceLink.hpp"
#include "AdvancedSettingsManager.hpp"
#include "AdminConsole.hpp"

#include <thread>


int main() {
    AdvancedSettingsManager::LoadSettings("server_data/control/config.json");
    AdminConsole::Init();

    std::thread connectionThread(&ServiceLink::StartTcpServer, AdvancedSettingsManager::GetSettings().port);
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

    AdvancedSettingsManager::SaveSettings();

    return 0;
}
