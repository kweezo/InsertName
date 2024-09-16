#pragma once

#include <string>
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
    static std::unique_ptr<pqxx::connection> c;
    static int logLevel;
    static int maxLogBufferSize;
    static std::vector<LogEntry> logsBuffer;

    static std::mutex mutex;
    static std::mutex dbMutex;
};

/* Alert levels:
 * 0: Useless
 * 1: Info
 * 2: Warning
 * 3: Error
 * 4: Critical
*/
