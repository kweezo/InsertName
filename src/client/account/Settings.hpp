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
    unsigned char anisotropy;
    bool anisotropyEnable;

    static Settings& GetInstance();
    void LoadConfig(const std::string& filename);
};
