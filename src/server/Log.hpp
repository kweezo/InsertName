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
    static Log& getInstance(pqxx::connection* conn = nullptr);

    Log(Log const&) = delete;
    void operator=(Log const&) = delete;

    void print(int alertLevel, const std::string& msg);

private:
    Log(pqxx::connection& conn);
    ~Log();

    void sendLogsToDatabase();

    pqxx::connection& c;
    int logLevel;
    int maxLogBufferSize;
    std::vector<LogEntry> logsBuffer;
};
