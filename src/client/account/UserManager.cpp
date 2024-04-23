#include "UserManager.hpp"


UserManager::UserManager(const std::string& serverIP, int port) : NetworkManager(serverIP, port) {}

int UserManager::loginUser(const char loginType,const std::string &username, const std::string &password) {
    std::string message = stickMessage(loginType, username, password);
    std::string response = sendData(message);
    int returnMsg = 0;
    if (returnMsg++, response == "r") { //1 user registered successfully
    } else if (returnMsg++, response == "l") { //2 user logged in successfully
    } else if (returnMsg++, response == "e") { //3 user already exists
    } else if (returnMsg++, response == "w") { //4 wrong password
    } else if (returnMsg++, response == "f") { //5 user not found
    } else { //6 error
        returnMsg++;
    }
    // maybe will add more cases

    return returnMsg;
}

std::string UserManager::getChat(const std::string& username, int offset) {
    std::string message = stickMessage('g', username, offset);
    std::string response = sendData(message);
    return response;
}

char UserManager::sendMessage(const std::string& receiverUsername, const std::string& message) {
    std::string msg = stickMessage('m', receiverUsername, message);
    std::string response = sendData(msg);
    return response[0];
}

std::string UserManager::getNewMessages(const std::string& username) {
    std::string message = stickMessage('n', username);
    std::string response = sendData(message);
    return response;
}

std::string UserManager::stickMessage(const char dataType, const std::string &str1, const std::string &str2) {
    return dataType + str1 + (char)30 + str2;
}

std::string UserManager::stickMessage(const char dataType, const std::string &str1, int num) {
    return dataType + str1 + (char)30 + std::to_string(num);
}

std::string UserManager::stickMessage(const char dataType, const std::string &str1) {
    return dataType + str1;
}

std::string UserManager::getNextArg(std::string& msg) {
    msg.erase(0, 1);

    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos); // Erase everything up to the next (char)30

    return arg;
}