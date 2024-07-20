#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <optional>
#include <regex>
#include <vector>
#include <variant>

#include <jsoncpp/json/json.h>

#include "Log.hpp"


struct Config {
    std::string controlServiceIp = "127.0.0.1";
    int controlServicePort = 8080;

    int logLevel = 0;
    int maxLogBufferSize = 128;

    std::string dbname = "postgres";
    std::string dbuser = "postgres";
    std::string dbpassword = "password";
    std::string dbhostaddr = "127.0.0.1";
    int dbport = 5432;

    std::string commandPrompt = "> ";
    int commandWindowHeight = 3;
};

class AdvancedSettingsManager {
public:
    static void LoadSettings(const std::string& file_);
    static void SaveSettings();
    static Config GetSettings();
    static void SetSettings(
        std::optional<std::string> controlSericeIp = std::nullopt,
        std::optional<int> controlServicePort = std::nullopt,

        std::optional<int> logLevel = std::nullopt,
        std::optional<int> maxLogBufferSize = std::nullopt,

        std::optional<std::string> dbname = std::nullopt,
        std::optional<std::string> dbuser = std::nullopt,
        std::optional<std::string> dbpassword = std::nullopt,
        std::optional<std::string> dbhostaddr = std::nullopt,
        std::optional<int> dbport = std::nullopt,

        std::optional<std::string> commandPrompt = std::nullopt,
        std::optional<int> commandWindowHeight = std::nullopt
    );
    static void SetSetting(size_t index, const std::variant<int, std::string>& value);
    static std::string GetSetting(size_t index);

    static bool TryPassDouble(const std::string& s, double& d);
    static bool TryPassInt(const std::string& s, int& i);
    static bool IsValidIPv4(const std::string& ip);

private:
    static std::string file;
    static Config config;
    static std::mutex mutex;
    static std::vector<std::variant<int*, std::string*>> settings;
};
