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
    Log(pqxx::connection& conn);
    ~Log();

    void print(int alertLevel, const std::string& msg);

private:

    void sendLogsToDatabase();

    pqxx::connection& c;
    int logLevel;
    int maxLogBufferSize;
    std::vector<LogEntry> logsBuffer;
};