#include "UserManager.hpp"


UserManager::UserManager() : NetworkManager("127.0.0.1", 12345) {
    connectToServer();
}

UserManager::~UserManager() {

}

std::string UserManager::stickMessage(const std::string &username, const std::string &password) {
    return (char)0 + username + (char)0 + password;
}

bool UserManager::registerUser(const std::string &username, const std::string &password) {

    return false;
}

bool UserManager::loginUser(const std::string &username, const std::string &password) {
    return false;
}
