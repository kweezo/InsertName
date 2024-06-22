#include "Config.hpp"

#include <iostream>
#include <regex>

std::string Config::dbname;
std::string Config::dbuser;
std::string Config::dbpassword;
std::string Config::dbhostaddr;
std::string Config::dbport;
int Config::serverPort;
int Config::loginAttempts;
int Config::logLevel;
int Config::maxLogBufferSize;
std::string Config::commandPrompt;
int Config::commandWindowHeight;
std::string Config::filename;
std::array<ConfigPointerVariant, 11> Config::configPointers;


void Config::InitializePointers() {
    configPointers = {
        &dbname,
        &dbuser,
        &dbpassword,
        &dbhostaddr,
        &dbport,
        &serverPort,
        &loginAttempts,
        &logLevel,
        &maxLogBufferSize,
        &commandPrompt,
        &commandWindowHeight
    };
}

std::string Config::AccessConfigPointer(size_t index) {
    if (index >= Config::configPointers.size()) {
        std::cerr << "Index out of range." << std::endl;
        return "";
    }

    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string*>) {
            return *arg;

        } else if constexpr (std::is_same_v<T, int*>) {
            return std::to_string(*arg);

        } else {
            return "Unsupported type";
        }
    }, Config::configPointers[index]);
}

void Config::SetConfigStringValue(size_t index, const std::string& value) {
    auto& variant = Config::configPointers[index];
    std::string** stringPtr = std::get_if<std::string*>(&variant);
    if (stringPtr) {
        **stringPtr = value;
    } else {
        std::cerr << "Index " << index << " does not point to a std::string type." << std::endl;
    }
}

void Config::SetConfigIntValue(size_t index, int value) {
    auto& variant = Config::configPointers[index];
    int** intPtr = std::get_if<int*>(&variant);
    if (intPtr) {
        **intPtr = value;
    } else {
        std::cerr << "Index " << index << " does not point to an int type." << std::endl;
    }
}

void Config::LoadConfig(const std::string& filename) {
    Config::filename = filename;
    std::unordered_map<std::string, std::string> settings;

    // Read the existing settings
    std::ifstream inFile(filename);
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            settings[key] = value;
        }
    }

    // Check for missing settings and add them
    std::ofstream outFile(filename, std::ios::app);
    if (settings.find("dbname") == settings.end()) {
        settings["dbname"] = "postgres";
        outFile << "dbname=postgres\n";
    }
    if (settings.find("dbuser") == settings.end()) {
        settings["dbuser"] = "postgres";
        outFile << "dbuser=postgres\n";
    }
    if (settings.find("dbpassword") == settings.end()) {
        settings["dbpassword"] = "password";
        outFile << "dbpassword=password\n";
    }
    if (settings.find("dbhostaddr") == settings.end()) {
        settings["dbhostaddr"] = "127.0.0.1";
        outFile << "dbhostaddr=127.0.0.1\n";
    }
    if (settings.find("dbport") == settings.end()) {
        settings["dbport"] = "5432";
        outFile << "dbport=5432\n";
    }
    if (settings.find("serverPort") == settings.end()) {
        settings["serverPort"] = "55832";
        outFile << "serverPort=55832\n";
    }
    if (settings.find("loginAttempts") == settings.end()) {
        settings["loginAttempts"] = "3";
        outFile << "loginAttempts=3\n";
    }
    if (settings.find("logLevel") == settings.end()) {
        settings["logLevel"] = "3";
        outFile << "logLevel=3\n";
    }
    if (settings.find("maxLogBufferSize") == settings.end()) {
        settings["maxLogBufferSize"] = "64";
        outFile << "maxLogBufferSize=64\n";
    }
    if (settings.find("commandPrompt") == settings.end()) {
        settings["commandPrompt"] = "> ";
        outFile << "commandPrompt=> \n";
    }
    if (settings.find("commandWindowHeight") == settings.end()) {
        settings["commandWindowHeight"] = "3";
        outFile << "commandWindowHeight=3\n";
    }

    if (!CheckSettingsFormat(settings)) {
        std::runtime_error("Invalid settings format");
    }

    // Now you can use the settings map to set your variables
    dbname = settings["dbname"];
    dbuser = settings["dbuser"];
    dbpassword = settings["dbpassword"];
    dbhostaddr = settings["dbhostaddr"];
    dbport = settings["dbport"];
    serverPort = std::stoi(settings["serverPort"]);
    loginAttempts = std::stoi(settings["loginAttempts"]);
    logLevel = std::stoi(settings["logLevel"]);
    maxLogBufferSize = std::stoi(settings["maxLogBufferSize"]);
    commandPrompt = settings["commandPrompt"];
    commandWindowHeight = std::stoi(settings["commandWindowHeight"]);
}

void Config::SaveConfig() {
    std::ofstream outFile(filename);

    outFile << "dbname=" << dbname << "\n";
    outFile << "dbuser=" << dbuser << "\n";
    outFile << "dbpassword=" << dbpassword << "\n";
    outFile << "dbhostaddr=" << dbhostaddr << "\n";
    outFile << "dbport=" << dbport << "\n";
    outFile << "serverPort=" << serverPort << "\n";
    outFile << "loginAttempts=" << loginAttempts << "\n";
    outFile << "logLevel=" << logLevel << "\n";
    outFile << "maxLogBufferSize=" << maxLogBufferSize << "\n";
    outFile << "commandPrompt=" << commandPrompt << "\n";
    outFile << "commandWindowHeight=" << commandWindowHeight << "\n";
}

bool Config::CheckSettingsFormat(std::unordered_map<std::string, std::string>& settings) {
    bool valid = true;
    if (!IsValidIPv4(settings["dbhostaddr"])) {
        std::cerr << "Invalid IPv4 address";
        valid = false;
    }

    int value;
    if (!TryPassInt(settings["dbport"], value) && (value < 1 || value > 65535)) {
        std::cerr << "Database port must be integer greater than 0 and smaller than 65536";
        valid = false;
    }
    if (!TryPassInt(settings["serverPort"], value) && (value < 1 || value > 65535)) {
        std::cerr << "Server port must be integer greater than  and smaller than 65536";
        valid = false;
    }
    if (!TryPassInt(settings["loginAttempts"], value) && value < 1) {
        std::cerr << "Login attempts must be integer greater than 0";
        valid = false;
    }
    if (!TryPassInt(settings["logLevel"], value) && (value < 0 || value > 4)) {
        std::cerr << "Log level must be integer between 0 and 4";
        valid = false;
    }
    if (!TryPassInt(settings["maxLogBufferSize"], value) && value < 1) {
        std::cerr << "Max log buffer size must be integer greater than 0";
        valid = false;
    }
    if (!TryPassInt(settings["commandWindowHeight"], value) && value < 1) {
        std::cerr << "Command window height must be integer greater than 0";
        valid = false;
    }

    return valid;
}

bool Config::TryPassDouble(const std::string& s, double& d) {
    std::istringstream iss(s);
    iss >> d;
    return iss.eof() && !iss.fail();
}

bool Config::TryPassInt(const std::string& s, int& i) {
    std::istringstream iss(s);
    iss >> i;
    return iss.eof() && !iss.fail();
}

bool Config::IsValidIPv4(const std::string& ip) {
    std::regex ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if (std::regex_match(ip, ipRegex)) {
        return true;
    }
    return false;
}