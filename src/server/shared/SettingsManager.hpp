#pragma once

#include <fstream>
#include <string>
#include <mutex>

#include <jsoncpp/json/json.h>


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
    static void CreateDefaultSettings();
    static bool SaveSettings(const std::string& configFile);

    static DefaultSettings settings;
    static std::mutex mutex;
};
