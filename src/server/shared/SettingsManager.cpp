#include "SettingsManager.hpp"

#include <jsoncpp/json/json.h>

#include <fstream>

DefaultSettings SettingsManager::settings;
std::mutex SettingsManager::mutex;


DefaultSettings SettingsManager::GetSettings() {
    std::lock_guard<std::mutex> lock(mutex);
    return settings;
}

bool SettingsManager::LoadSettings(const std::string& configFile) {
    std::lock_guard<std::mutex> lock(mutex);

    std::ifstream file(configFile);
    if (!file.is_open()) {
        CreateDefaultSettings();
        SaveSettings(configFile);
        return true;
    }
    Json::Reader reader;
    Json::Value root;
    bool parsingSuccessful = reader.parse(file, root);
    file.close();

    if (!parsingSuccessful) {
        CreateDefaultSettings();
        SaveSettings(configFile);
        return true;
    }

    bool needSave = false;
    if (!root.isMember("ip")) {
        settings.ip = "127.0.0.1";
        needSave = true;
    } else {
        settings.ip = root["ip"].asString();
    }

    if (!root.isMember("port")) {
        settings.port = 8080;
        needSave = true;
    } else {
        settings.port = root["port"].asInt();
    }

    if (!root.isMember("serviceId")) {
        settings.serviceId = 1;
        needSave = true;
    } else {
        settings.serviceId = root["serviceId"].asInt();
    }

    if (needSave) {
        SaveSettings(configFile);
    }

    return true;
}

void SettingsManager::CreateDefaultSettings() {
    settings.ip = "127.0.0.1";
    settings.port = 8080;
    settings.serviceId = 1;
}

bool SettingsManager::SaveSettings(const std::string& configFile) {
    std::ofstream file(configFile);
    if (!file.is_open()) {
        return false;
    }

    Json::Value root;
    root["ip"] = settings.ip;
    root["port"] = settings.port;
    root["serviceId"] = settings.serviceId;

    Json::StyledWriter writer;
    file << writer.write(root);
    file.close();
    return true;
}
