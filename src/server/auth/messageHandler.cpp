#include "messageHandler.hpp"

#include "Settings.hpp"
#include "ClientHandler.hpp"
#include "common/TypeUtils.hpp"
#include "shared/ClientServiceLink.hpp"

#include <iostream>


void handleMessageContent(std::string message) {
    std::string action = TypeUtils::getFirstParam(message);

    if (action == "SETTING_SET_dbConnString") {
        settings.dbConnString = message;
        allSettingsReceivedArray[0] = true;

    } else if (action == "SETTING_SET_port") {
        settings.port = std::stoi(message);
        allSettingsReceivedArray[1] = true;

    } else if (action == "SETTING_SET_EMAIL_VERIFICATIONS_ATTEMPTS") {
        settings.emailVerificationsAttempts = std::stoi(message);
        allSettingsReceivedArray[2] = true;

    } else if (action == "SHUTTING_DOWN") {
        ClientHandler::InitiateShutdown();

    } else if (action == "SHUTDOWN") {
        ClientHandler::Shutdown();
        ClientServiceLink::running = false;
        
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
    }
}
