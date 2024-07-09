#include "ServiceLink.hpp"
#include "AdvancedSettingsManager.hpp"

#include <thread>


int main() {
    AdvancedSettingsManager::LoadSettings("server_data/control/config.json");

    std::thread connectionThread(&ServiceLink::StartTcpServer, AdvancedSettingsManager::GetSettings().port);
    std::thread messageProcessingThread(&ServiceLink::ProcessMessages);
    std::thread messageSendingThread(&ServiceLink::ProcessSendBuffer);


    // ----------------- Add here some other code ----------------- //


    messageSendingThread.join();
    connectionThread.join();
    messageProcessingThread.join();

    AdvancedSettingsManager::SaveSettings();

    return 0;
}
