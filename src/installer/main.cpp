#include "../settings.hpp"

int main() {
    Settings settings;
    settings.width = 1920;  // Set the width
    settings.height = 1080; // Set the height
    // Set other settings as needed

    WriteSettings(settings, "../src/settings.bin");

    return 0;
}