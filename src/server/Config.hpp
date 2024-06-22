#pragma once

#include <array>
#include <string>
#include <variant>
#include <fstream>
#include <sstream>
#include <unordered_map>


using ConfigPointerVariant = std::variant<std::string*, int*>;

class Config {
public:
    static std::array<ConfigPointerVariant, 11> configPointers;
    static std::string AccessConfigPointer(size_t index);
    static void SetConfigStringValue(size_t index, const std::string& value);
    static void SetConfigIntValue(size_t index, int value);

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

    static void InitializePointers();
    
    static bool TryPassDouble(const std::string& s, double& d);
    static bool TryPassInt(const std::string& s, int& i);
    static bool IsValidIPv4(const std::string& ip);

private:
    static bool CheckSettingsFormat(std::unordered_map<std::string, std::string>& settings);
};
