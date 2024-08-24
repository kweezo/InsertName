#pragma once

#include "NetworkManager.hpp"
#include <string>
#include <glm/glm.hpp>

class UserManager : public NetworkManager {
public:
    UserManager(const std::string& serverIP, int port);

    // loginType = 'r' for register, 'l' for login
    int loginUser(const unsigned char loginType, const std::string &username, const std::string &password);
    std::string getChat(const std::string& username, int offset = 0);
    char sendMessage(const std::string& receiverUsername, const std::string& message);
    std::string getNewMessages(const std::string& username);

    char sendStruct(glm::vec3 v1[8], glm::vec3 v2[8]);

    std::string getNextArg(std::string& msg);
    std::string stickMessage(const std::string &str1, const std::string &str2);
    std::string stickMessage(const std::string &str1, int num);
};