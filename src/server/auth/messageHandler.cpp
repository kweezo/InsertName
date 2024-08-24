#include "messageHandler.hpp"


void handleMessageContent(std::string message) {
    std::string action = ClientServiceLink::GetFirstParameter(message);

    if (action == "SETTING_GET_dbConnString") {
        settings.dbConnString = message;
        allSettingsReceivedArray[0] = true;

    } else if (action == "SETTING_GET_port") {
        settings.port = std::stoi(message);
        allSettingsReceivedArray[1] = true;
        
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
    }
}
