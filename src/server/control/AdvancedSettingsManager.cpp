#include "AdvancedSettingsManager.hpp"

std::string AdvancedSettingsManager::file;
Config AdvancedSettingsManager::config;
std::mutex AdvancedSettingsManager::mutex;


void AdvancedSettingsManager::LoadSettings(const std::string& file_) {
    std::lock_guard<std::mutex> lock(mutex);
    file = file_;

    std::ifstream configFile(file, std::ifstream::binary);
    if (configFile) {
        std::cout << "Loading settings from " << file << std::endl;
        Json::Value root;
        configFile >> root;

        if (root.isMember("serviceId")) {
            config.serviceId = root["serviceId"].asInt();
        }
        if (root.isMember("port")) {
            config.port = root["port"].asInt();
        }
    }
}

void AdvancedSettingsManager::SaveSettings() {
    std::lock_guard<std::mutex> lock(mutex);

    Json::Value root;
    root["serviceId"] = config.serviceId;
    root["port"] = config.port;

    std::ofstream configFile(file, std::ofstream::binary);
    configFile << root;

    std::cout << "Settings saved to " << file << std::endl;
}

Config AdvancedSettingsManager::GetSettings() {
    std::lock_guard<std::mutex> lock(mutex);
    return config;
}

void AdvancedSettingsManager::SetSettings(std::optional<int> serviceId, std::optional<int> port) {
    std::lock_guard<std::mutex> lock(mutex);

    if (serviceId.has_value()) {
        config.serviceId = serviceId.value();
    }
    if (port.has_value()) {
        config.port = port.value();
    }
}
