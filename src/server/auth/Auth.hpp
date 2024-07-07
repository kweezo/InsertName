#pragma once

#include <string>
#include <pqxx/pqxx>

class Auth {
public:
    static bool Init();

    static int RegisterUser(const std::string& username, const std::string& password, const std::string& email);
    static bool LoginUser(const std::string& username, const std::string& password);

private:
    static std::string HashPassword(const std::string& password, const std::string& salt);
    static std::string GenerateSalt();

    static pqxx::connection* conn;
};
