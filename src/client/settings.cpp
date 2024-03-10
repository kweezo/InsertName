#include "settings.hpp"

void readSettings(Settings& settings, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    in.read(reinterpret_cast<char*>(&settings), sizeof(Settings));
}

void writeSettings(const Settings& settings, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
}