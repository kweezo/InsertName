#include <fstream>
#include <string>

struct Settings {
    int width;
    int height;
};

void ReadSettings(Settings& settings, const std::string& filename);
void WriteSettings(const Settings& settings, const std::string& filename);