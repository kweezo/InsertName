#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

class HandleClient {
public:
    HandleClient(const std::string& directory);

    std::string handleMsg(const std::string& receivedMsg);
    char registerUser(const std::string& username, const std::string& password);
    std::string getNextArg(std::string& msg);

private:
    std::string username;
    static std::string dir;
    std::string userDir;
};