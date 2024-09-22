#pragma once

#include <string>

#define SETTINGS_NUM 6

struct Settings {
    std::string dbConnString;
    unsigned short port;
    unsigned short emailVerificationsAttempts;
    unsigned short loginAttempts;
    unsigned short loginTime;
    unsigned short emailVerificationTime;
};


bool allSettingsReceived();

extern bool allSettingsReceivedArray[SETTINGS_NUM];
extern Settings settings;
