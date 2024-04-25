#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

class Settings {
public:
    int windowWidth;
    int windowHeight;
    std::string serverIP;
    int serverPort;
    int messageBufferSize;
    unsigned char anisotropy;
    bool anisotropyEnable;

    std::string filename;
    static Settings& GetInstance();
    void LoadSettings(const std::string& filename);
    void SaveSettings();
};
