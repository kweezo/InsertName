#pragma once

#include "../network/NetworkManager.hpp"

#include <string>

class UserManager : public NetworkManager {
public:
    UserManager();
    ~UserManager();

    bool registerUser(const std::string& username, const std::string& password);
    bool loginUser(const std::string& username, const std::string& password);

private:
    std::string stickMessage(const std::string& username, const std::string& password);
};