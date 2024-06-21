#pragma once

#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <unordered_map>


class Config {
public:
    static std::string dbname;
    static std::string dbuser;
    static std::string dbpassword;
    static std::string dbhostaddr;
    static std::string dbport;

    static int serverPort;

    static int loginAttempts;
    static int logLevel;
    static int maxLogBufferSize;
    static std::string commandPrompt;
    static int commandWindowHeight;

    static std::string filename;
    static void LoadConfig(const std::string& filename);
    static void SaveConfig();

    static std::array<void*, 11> configPointers;
    static void InitializePointers();
    
    static bool IsDouble(const std::string& s, double& d);
    static bool IsInt(const std::string& s, int& i);
    static bool IsValidIPv4(const std::string& ip);

private:
    static bool CheckSettingsFormat(std::unordered_map<std::string, std::string>& settings);
};
