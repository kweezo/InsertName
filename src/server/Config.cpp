#include "Config.hpp"

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

void Config::LoadConfig(const std::string& filename) {
    this->filename = filename;
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
    if (settings.find("commandPrefix") == settings.end()) {
        settings["commandPrefix"] = "> ";
        outFile << "commandPrefix=> \n";
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
    commandPrefix = settings["commandPrefix"];
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
    outFile << "commandPrefix=" << commandPrefix << "\n";
}