#pragma once

#include <string>

#define SETTINGS_NUM 3

struct Settings {
    std::string dbConnString;
    unsigned short port;
    unsigned short emailVerificationsAttempts;
};


bool allSettingsReceived();

extern bool allSettingsReceivedArray[SETTINGS_NUM];
extern Settings settings;
