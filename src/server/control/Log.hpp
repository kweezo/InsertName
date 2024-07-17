#pragma once

#include "AdminConsole.hpp"
#include "AdvancedSettingsManager.hpp"

#include <ctime>
#include <string>
#include <iostream>
#include <vector>

#include <pqxx/pqxx>

struct LogEntry {
    std::time_t timestamp;
    int alertLevel;
    std::string message;
};

class Log {
public:
    static void Init();
    static void Destroy();

    static void Print(int alertLevel, const std::string& msg);
    static void SendLogsToDatabase();

private:
    static std::unique_ptr<pqxx::connection> c;
    static int logLevel;
    static int maxLogBufferSize;
    static std::vector<LogEntry> logsBuffer;
};