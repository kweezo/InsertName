#include <fstream>

struct Settings {
    int width;
    int height;
};

void readSettings(Settings& settings, const std::string& filename = "settings.bin");
void writeSettings(const Settings& settings, const std::string& filename);