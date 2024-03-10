#include "settings.hpp"

void readSettings(Settings& settings, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    in.read(reinterpret_cast<char*>(&settings), sizeof(Settings));
}

void writeSettings(const Settings& settings, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
}

void writeSettingsIfNotSet(const Settings& settings, const std::string& filename) {
    std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);

    // If file does not exist, create it
    if (!file.is_open()) {
        file.open(filename, std::ios::binary | std::ios::out);
    }

    int existingSetting;
    file.seekg(0);
    file.read(reinterpret_cast<char*>(&existingSetting), sizeof(int));
    if (file.gcount() != sizeof(int)) { // If the setting is not set, write it
        file.seekp(0);
        file.write(reinterpret_cast<const char*>(&settings.width), sizeof(int));
    }

    file.seekg(sizeof(int));
    file.read(reinterpret_cast<char*>(&existingSetting), sizeof(int));
    if (file.gcount() != sizeof(int)) { // If the setting is not set, write it
        file.seekp(sizeof(int));
        file.write(reinterpret_cast<const char*>(&settings.height), sizeof(int));
    }

    // Repeat the process for each setting in the Settings struct

    file.close();
}