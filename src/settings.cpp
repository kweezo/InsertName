#include "settings.hpp"

void ReadSettings(Settings& settings, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open settings file for reading");
    }

    in.read(reinterpret_cast<char*>(&settings), sizeof(Settings));
    if (in.fail()) {
        throw std::runtime_error("Failed to read settings from file");
    }
}

void WriteSettings(const Settings& settings, const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        // If the file does not exist, create it by opening it for writing
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to create settings file");
        }

        // Write the settings to the file
        outFile.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
        if (outFile.fail()) {
            throw std::runtime_error("Failed to write to the settings file");
        }
    } else {
        // If the file does exist, check if the settings are already set
        Settings existingSettings;
        inFile.read(reinterpret_cast<char*>(&existingSettings), sizeof(Settings));

        // If the settings are not set, write the new settings to the file
        if (inFile.gcount() != sizeof(Settings)) {
            std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
            if (!outFile) {
                throw std::runtime_error("Failed to open settings file for writing");
            }

            outFile.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
            if (outFile.fail()) {
                throw std::runtime_error("Failed to write to the settings file");
            }
        }
    }
}