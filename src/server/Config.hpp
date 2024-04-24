#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <map>


class Config {
public:
    std::string dbname;
    std::string dbuser;
    std::string dbpassword;
    std::string dbhostaddr;
    std::string dbport;

    int messageBatchSize;
    int messageBufferSize;

    static Config& GetInstance();
    void LoadConfig(const std::string& filename);
};
