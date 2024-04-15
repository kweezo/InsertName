#include "Config.hpp"


Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

void Config::LoadConfig(const std::string& filename) {
    std::fstream file(filename, std::ios::in | std::ios::out);
    if (!file.is_open()) {
        // If file does not exist, create it and write default values
        file.open(filename, std::ios::out);
        file << "dbname=postgres\n";
        file << "dbuser=postgres\n";
        file << "dbpassword=password\n";
        file << "dbhostaddr=127.0.0.1\n";
        file << "dbport=5432\n";
        file.close();
        // Reopen the file for reading
        file.open(filename, std::ios::in);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                if (key == "dbname") {
                    dbname = value;
                } else if (key == "dbuser") {
                    dbuser = value;
                } else if (key == "dbpassword") {
                    dbpassword = value;
                } else if (key == "dbhostaddr") {
                    dbhostaddr = value;
                } else if (key == "dbport") {
                    dbport = value;
                }
            }
        }
    }
}
