#include <fstream>
#include <string>

struct Settings {
    int width;
    int height;
};

void readSetting(int& setting, const std::string& settingName, const std::string& filename = "settings.bin");
void writeSetting(const int& setting, const std::string& settingName, const std::string& filename = "settings.bin");
void writeSettingsIfNotSet(const Settings& settings, const std::string& filename);