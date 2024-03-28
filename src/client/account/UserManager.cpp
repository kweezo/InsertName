#include "UserManager.hpp"


UserManager::UserManager(const std::string& serverIP, int port) : NetworkManager(serverIP, port) {

}

UserManager::~UserManager() {

}

int UserManager::loginUser(const char loginType,const std::string &username, const std::string &password) {
    std::string message = stickMessage(loginType, username, password);
    std::string response = sendData(message);
    int returnMsg = 0;
    if (returnMsg++, response == "r") { //1 user registered successfully
    } else if (returnMsg++, response == "e") { //2 user already exists
    } else if (returnMsg++, response == "l") { //3 user logged in successfully
    } else if (returnMsg++, response == "w") { //4 wrong password
    } else if (returnMsg++, response == "f") { //5 user not found
    } else { //6 error
        returnMsg++;
    }
    // maybe will add more cases

    return returnMsg;
}

std::string UserManager::stickMessage(const char dataType, const std::string &str1, const std::string &str2) {
    return dataType + str1 + (char)30 + str2;
}