#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <SQLite3/sqlite3.h>

class HandleClient {
public:
    HandleClient(const std::string& directory);
    ~HandleClient();

    std::string handleMsg(const std::string& receivedMsg);
    char registerUser(const std::string& username, const std::string& password);
    char loginUser(const std::string& username, const std::string& password);
    std::string getNextArg(std::string& msg);

private:
    std::string username;
    static std::string dir;
    std::string userDir;

    sqlite3* db = nullptr;
};