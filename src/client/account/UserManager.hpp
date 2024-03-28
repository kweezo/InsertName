#pragma once

#include "NetworkManager.hpp"
#include <string>

class UserManager : public NetworkManager {
public:
    UserManager(const std::string& serverIP, int port);
    ~UserManager();

    int loginUser(const char loginType,const std::string &username, const std::string &password); // loginType = 'r' for register, 'l' for login

    std::string stickMessage(const char dataType, const std::string &str1, const std::string &str2);
private:

};