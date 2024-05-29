#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>


class Config {
public:
    std::string dbname;
    std::string dbuser;
    std::string dbpassword;
    std::string dbhostaddr;
    std::string dbport;

    int serverPort;

    int loginAttempts;
    int logLevel;
    int maxLogBufferSize;
    std::string commandPrefix;

    std::string filename;
    static Config& GetInstance();
    void LoadConfig(const std::string& filename);
    void SaveConfig();
};
