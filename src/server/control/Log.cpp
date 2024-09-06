#include "Log.hpp"

std::unique_ptr<pqxx::connection> Log::c;
int Log::logLevel;
int Log::maxLogBufferSize;
std::vector<LogEntry> Log::logsBuffer;
std::mutex Log::mutex;
std::mutex Log::dbMutex;


void Log::Init() {
    std::lock_guard<std::mutex> lock(mutex);
    logLevel = AdvancedSettingsManager::GetSettings().logLevel;
    maxLogBufferSize = AdvancedSettingsManager::GetSettings().maxLogBufferSize;

    std::lock_guard<std::mutex> dbLock(dbMutex);
    std::string conn_str = "dbname=" + AdvancedSettingsManager::GetSettings().dbname +
                          " user=" + AdvancedSettingsManager::GetSettings().dbuser +
                          " password=" + AdvancedSettingsManager::GetSettings().dbpassword +
                          " hostaddr=" + AdvancedSettingsManager::GetSettings().dbhostaddr +
                          " port=" + std::to_string(AdvancedSettingsManager::GetSettings().dbport);
    c = std::make_unique<pqxx::connection>(conn_str); 
    
    if (!c->is_open()) {
        throw std::runtime_error("Database connection is not open");
    }
    // Create logs table if it doesn't exist
    pqxx::work w(*c);
    w.exec("CREATE TABLE IF NOT EXISTS logs (timestamp DOUBLE PRECISION, alert_level INT, message TEXT)");
    w.commit();
}

void Log::Destroy() {
    std::lock_guard<std::mutex> lock(mutex);
    // Send all remaining logs to the database
    SendLogsToDatabase();

    c->close();
    c = nullptr;
}

void Log::Print(const std::string& msg, int alertLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    // Get current time as custom timestamp
    double now = GetCurrentTimestamp();

    if (alertLevel > 3-logLevel) {
        int colorPair = alertLevel+1;
    
        // Format the timestamp
        std::time_t now_time_t = std::time(nullptr);
        std::tm* tm = std::localtime(&now_time_t);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%d.%m %H:%M:%S", tm);
    
        // Construct and print the log message
        std::string logMsg = "[" + std::string(buffer) + "] " + msg;
        AdminConsole::PrintLog(logMsg, colorPair);
    }

    // Add log to the buffer
    logsBuffer.push_back({now, alertLevel, msg});

    // If we have enough logs in the buffer, send them to the database
    if (logsBuffer.size() >= maxLogBufferSize) {
        SendLogsToDatabase();
    }
}

void Log::SendLogsToDatabase() {
    std::lock_guard<std::mutex> lock(dbMutex);
    // Create log entries
    pqxx::work w(*c);
    for (const auto& log : logsBuffer) {
        std::string str_msg(log.message.begin(), log.message.end());
        w.exec_params("INSERT INTO logs (timestamp, alert_level, message) VALUES ($1, $2, $3)", log.timestamp, log.alertLevel, str_msg);
    }
    w.commit();

    // Clear the buffer
    logsBuffer.clear();
}

double Log::GetCurrentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_duration = now.time_since_epoch();
    auto now_seconds = duration_cast<seconds>(now_duration).count();
    auto now_microseconds = duration_cast<microseconds>(now_duration).count() % 1000000;

    int days_since_epoch = now_seconds / 86400;
    double fractional_day = (now_seconds % 86400 * 1000000 + now_microseconds) / (86400.0 * 1000000);

    return days_since_epoch + fractional_day;
}
