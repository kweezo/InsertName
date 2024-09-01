#include "../settings.hpp"

#define VK_KHR_external_memory_win32 

int main() {
    Settings settings;
    settings.width = 1920;
    settings.height = 1080;

    WriteSettings(settings, "src/settings.bin");

    return 0;
}