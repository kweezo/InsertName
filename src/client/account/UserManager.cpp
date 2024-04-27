#include "UserManager.hpp"


UserManager::UserManager(const std::string& serverIP, int port) : NetworkManager(serverIP, port) {}

char UserManager::sendStruct(glm::vec3 v1[8], glm::vec3 v2[8]) {
    std::string data;

    // Add the binary data for v1 and v2
    for (int i = 0; i < 8; i++) {
        data.append(reinterpret_cast<const char*>(&v1[i]), sizeof(glm::vec3));
    }
    for (int i = 0; i < 8; i++) {
        data.append(reinterpret_cast<const char*>(&v2[i]), sizeof(glm::vec3));
    }

    // Send the data with identifier 's'
    std::string response = sendData(static_cast<unsigned char>('s'), data);

    return response[0];
}

int UserManager::loginUser(const unsigned char loginType,const std::string &username, const std::string &password) {
    std::string message = stickMessage(username, password);
    std::string response = sendData(loginType, message);
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

std::string UserManager::getChat(const std::string& username, int offset) { //? change second argument to int
    std::string message = stickMessage(username, offset);
    std::string response = sendData(static_cast<unsigned char>('g'), message);
    return response;
}

char UserManager::sendMessage(const std::string& receiverUsername, const std::string& message) {
    std::string msg = stickMessage(receiverUsername, message);
    std::string response = sendData(static_cast<unsigned char>('m'), msg);
    return response[0];
}

std::string UserManager::getNewMessages(const std::string& username) {
    std::string response = sendData(static_cast<unsigned char>('n'), username);
    return response;
}

std::string UserManager::stickMessage(const std::string &str1, const std::string &str2) {
    return str1 + (char)30 + str2;
}

std::string UserManager::stickMessage(const std::string &str1, int num) {
    return str1 + (char)30 + std::to_string(num);
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