#include "Settings.hpp"

Settings& Settings::GetInstance() {
    static Settings instance;
    return instance;
}

void Settings::LoadSettings(const std::string& filename) {
    this->filename = filename;
    std::unordered_map<std::string, std::string> settings;

    // Read the existing settings
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            settings[key] = value;
        }
    }

    // Check for missing settings and add them
    std::ofstream outFile(filename, std::ios::app);
    if (settings.find("windowWidth") == settings.end()) {
        settings["windowWidth"] = "1920";
        outFile << "windowWidth=1920\n";
    }
    if (settings.find("windowHeight") == settings.end()) {
        settings["windowHeight"] = "1080";
        outFile << "windowHeight=1080\n";
    }
    if (settings.find("serverIP") == settings.end()) {
        settings["serverIP"] = "127.0.0.1";
        outFile << "serverIP=127.0.0.1\n";
    }
    if (settings.find("serverPort") == settings.end()) {
        settings["serverPort"] = "12345";
        outFile << "serverPort=12345\n";
    }
    if (settings.find("messageBufferSize") == settings.end()) {
        settings["messageBufferSize"] = "1024";
        outFile << "messageBufferSize=1024\n";
    }
    if (settings.find("anisotropy") == settings.end()) {
        settings["anisotropy"] = "2";
        outFile << "anisotropy=2\n";
    }
    if (settings.find("anisotropyEnable") == settings.end()) {
        settings["anisotropyEnable"] = "true";
        outFile << "anisotropyEnable=true\n";
    }

    // Now you can use the settings map to set your variables
    windowWidth = std::stoi(settings["windowWidth"]);
    windowHeight = std::stoi(settings["windowHeight"]);
    serverIP = settings["serverIP"];
    serverPort = std::stoi(settings["serverPort"]);
    messageBufferSize = std::stoi(settings["messageBufferSize"]);
    anisotropy = std::stoi(settings["anisotropy"]);
    anisotropyEnable = settings["anisotropyEnable"] == "true";
}

void Settings::SaveSettings() {
    std::ofstream outFile(filename);

    // Write the current settings to the file
    outFile << "windowWidth=" << windowWidth << "\n";
    outFile << "windowHeight=" << windowHeight << "\n";
    outFile << "serverIP=" << serverIP << "\n";
    outFile << "serverPort=" << serverPort << "\n";
    outFile << "messageBufferSize=" << messageBufferSize << "\n";
    outFile << "anisotropy=" << anisotropy << "\n";
    outFile << "anisotropyEnable=" << (anisotropyEnable ? "true" : "false") << "\n";
}