#pragma once

#include "buildConfig.hpp"
#include "Config.hpp"

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
    static void init();
    static void destroy();

    static void print(int alertLevel, const std::string& msg);


private:
    static void sendLogsToDatabase();

    static std::unique_ptr<pqxx::connection> c;
    static int logLevel;
    static int maxLogBufferSize;
    static std::vector<LogEntry> logsBuffer;
};