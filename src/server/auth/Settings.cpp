#include "Settings.hpp"

Settings settings;
bool allSettingsReceivedArray[SETTINGS_NUM] = {false, false, false};

bool allSettingsReceived() {
    bool yes = true;
    for (int i = 0; i < SETTINGS_NUM; i++) {
        yes = yes && allSettingsReceivedArray[i];
    }
    return yes;    
}
