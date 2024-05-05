#include "Log.hpp"


Log::Log(pqxx::connection& conn) :
    c(conn),
    logLevel(Config::GetInstance().logLevel), 
    maxLogBufferSize(Config::GetInstance().maxLogBufferSize) {
#ifndef NO_DB
    if (!c.is_open()) {
        throw std::runtime_error("Database connection is not open (First time you need to provide a open connection to the database)");
    }
    // Create logs table if it doesn't exist
    pqxx::work w(c);
    w.exec("CREATE TABLE IF NOT EXISTS logs (timestamp INT, alert_level INT, message TEXT)");
    w.commit();
#endif
}

Log::~Log() {
#ifndef NO_DB
    // Send all remaining logs to the database
    sendLogsToDatabase();
#endif
}

Log& Log::getInstance(pqxx::connection* conn) {
    static Log* instance = nullptr;
    static pqxx::connection* stored_conn = nullptr;
    if (!instance) {
        if (!conn) {
            throw std::runtime_error("Must provide a connection for the first call to getInstance");
        }
        stored_conn = conn;
        instance = new Log(*stored_conn);
    }
    return *instance;
}

void Log::print(int alertLevel, const std::string& msg) {
    // Get current time as Unix timestamp
    std::time_t now = std::time(nullptr);

    if (alertLevel > 2-logLevel) {
        int colorPair;
        if (alertLevel == 0) {
            colorPair = 1; // White on black
        } else if (alertLevel == 1) {
            colorPair = 2; // Yellow on black
        } else if (alertLevel == 2) {
            colorPair = 3; // Red on black
        }
    
        // Format the timestamp
        std::tm* tm = std::localtime(&now);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%d.%m %H:%M:%S", tm);
    
        // Construct and print the log message
        std::string logMsg = "[" + std::string(buffer) + "] " + msg;
        AdminConsole::printLog(logMsg, colorPair);
    }

#ifndef NO_DB
    // Add log to the buffer
    logsBuffer.push_back({now, alertLevel, msg});

    // If we have enough logs in the buffer, send them to the database
    if (logsBuffer.size() >= maxLogBufferSize) {
        sendLogsToDatabase();
    }
#endif
}

void Log::sendLogsToDatabase() {
    // Create log entries
    pqxx::work w(c);
    for (const auto& log : logsBuffer) {
        std::string str_msg(log.message.begin(), log.message.end());
        w.exec_params("INSERT INTO logs (timestamp, alert_level, message) VALUES ($1, $2, $3)", log.timestamp, log.alertLevel, str_msg);
    }
    w.commit();

    // Clear the buffer
    logsBuffer.clear();
}
