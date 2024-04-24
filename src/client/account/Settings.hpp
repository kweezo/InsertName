#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <map>

class Settings {
public:
    int windowWidth;
    int windowHeight;
    std::string serverIP;
    int serverPort;

    static Settings& GetInstance();
    void LoadConfig(const std::string& filename);
};
