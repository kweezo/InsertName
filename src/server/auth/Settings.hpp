#pragma once

#include <string>

#define SETTINGS_NUM 2

struct Settings {
    std::string dbConnString;
    unsigned short port;
};


bool allSettingsReceived();

extern bool allSettingsReceivedArray[SETTINGS_NUM];
extern Settings settings;
