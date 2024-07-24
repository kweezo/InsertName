#pragma once

#include "AdminConsole.hpp"
#include "AdvancedSettingsManager.hpp"

#include <ctime>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#include <pqxx/pqxx>

struct LogEntry {
    double timestamp;
    int alertLevel;
    std::string message;
};

class Log {
public:
    static void Init();
    static void Destroy();

    static void Print(const std::string& msg, int alertLevel);
    static void SendLogsToDatabase();

private:
    static double GetCurrentTimestamp();

    static std::unique_ptr<pqxx::connection> c;
    static int logLevel;
    static int maxLogBufferSize;
    static std::vector<LogEntry> logsBuffer;

    static std::mutex mutex;
    static std::mutex dbMutex;
};
