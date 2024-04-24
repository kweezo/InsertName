#include "Config.hpp"


Config& Config::GetInstance() {
    static Config instance;
    return instance;
}
void Config::LoadConfig(const std::string& filename) {
    std::map<std::string, std::string> settings;

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
    if (settings.find("messageBatchSize") == settings.end()) {
        settings["messageBatchSize"] = "30";
        outFile << "messageBatchSize=30\n";
    }
    if (settings.find("messageBufferSize") == settings.end()) {
        settings["messageBufferSize"] = "4096";
        outFile << "messageBufferSize=4096\n";
    }

    // Now you can use the settings map to set your variables
    dbname = settings["dbname"];
    dbuser = settings["dbuser"];
    dbpassword = settings["dbpassword"];
    dbhostaddr = settings["dbhostaddr"];
    dbport = settings["dbport"];
    messageBatchSize = std::stoi(settings["messageBatchSize"]);
    messageBufferSize = std::stoi(settings["messageBufferSize"]);
}