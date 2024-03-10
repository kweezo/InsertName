#include "../settings.hpp"

int main() {
    Settings settings;
    settings.width = 1920;
    settings.height = 1080;

    WriteSettings(settings, "../src/settings.bin");

    return 0;
}