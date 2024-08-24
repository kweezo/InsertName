#include "Settings.hpp"

Settings settings;
bool allSettingsReceivedArray[SETTINGS_NUM] = {false, false};

bool allSettingsReceived() {
    bool yes = false;
    for (int i = 0; i < SETTINGS_NUM; i++) {
        yes = yes && allSettingsReceivedArray[i];
    }
    return yes;    
}
