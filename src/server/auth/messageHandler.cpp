#include "messageHandler.hpp"


void handleMessageContent(std::string message) {
    std::string action = ClientServiceLink::GetFirstParameter(message);

    if (action == "SETTING_GET_dbConnString") {
        settings.dbConnString = message;
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
    }
}
