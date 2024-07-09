#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <optional>
#include <iostream>

#include <jsoncpp/json/json.h>


struct Config {
    int serviceId = 0;
    int port = 8080;
};

class AdvancedSettingsManager {
public:
    static void LoadSettings(const std::string& file_);
    static void SaveSettings();
    static Config GetSettings();
    static void SetSettings(std::optional<int> serviceId = std::nullopt, std::optional<int> port = std::nullopt);

private:
    static std::string file;
    static Config config;
    static std::mutex mutex;
};
