#include "settings.hpp"

void readSettings(Settings& settings, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    in.read(reinterpret_cast<char*>(&settings), sizeof(Settings));
}

void writeSettings(const Settings& settings, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
}
#include <iostream>

void WriteSettingsIfNotSet(const Settings& settings, const std::string& filename) {
    std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    int existingSetting;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&existingSetting), sizeof(int));
    if (file.gcount() != sizeof(int)) { 
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(&settings.width), sizeof(int));
        if(file.fail()){
            throw std::runtime_error("Failed to write to the settings file");
        }
        std::cout << "Writing width" << std::endl;
    }

    file.seekg(sizeof(int));
    file.read(reinterpret_cast<char*>(&existingSetting), sizeof(int));
    if (file.gcount() != sizeof(int)) { 
        file.seekp(sizeof(int));
        file.write(reinterpret_cast<const char*>(&settings.height), sizeof(int));
    }

    file.close();
}