#pragma once

#include <jsoncpp/json/json.h>
#include <fstream>
#include <string>


struct DefaultSettings {
    std::string ip;
    int port;
    int serviceId;
};

class SettingsManager {
public:
    static bool LoadSettings(const std::string& configFile);

    static DefaultSettings GetSettings();

private:
    static DefaultSettings settings;

    static void CreateDefaultSettings();
    static bool SaveSettings(const std::string& configFile);
};
