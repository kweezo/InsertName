#pragma once

#include <string>
#include <iostream>

#include <pqxx/pqxx>

class HandleClient {
public:
    HandleClient(const std::string& directory);
    ~HandleClient();

    std::string handleMsg(const std::string& receivedMsg);
    char registerUser(const std::string& username, const std::string& password);
    char loginUser(const std::string& username, const std::string& password);
    std::string getNextArg(std::string& msg);

private:
    static std::string dir;
    pqxx::connection c;
    std::string username;
};