#pragma once

#include <string>
#include <pqxx/pqxx>


struct UserRegistration {
    std::string username;
    std::string password;
    std::string email;

    bool validUsername = false;
    bool validPassword = false;
    bool validEmail = false;
};

class Auth {
public:
    static void Init();

    static bool RegisterUser(const std::string& username, const std::string& password, const std::string& email);
    static bool LoginUser(int uid, const std::string& password);
    static bool LoginUser(int uid);
    static void LogoutUser(int uid);
    static bool ChangePassword(int uid, const std::string& oldPassword, const std::string& newPassword);

    static bool VerifyPassword(int uid, const std::string& password);
    static bool VerifyEmail(const std::string& email);
    static bool CheckUsername(const std::string& username);
    static bool CheckPassword(const std::string& password);
    static bool CheckEmail(const std::string& email);

    static bool SendReloginToken(int uid);
    static bool VerifyReloginToken(int uid, const std::string& token);

    static bool SendLoginToken(int uid);

    static int GetUID(const std::string& username);

private:
    static std::string HashString(const std::string& string, const std::string& salt);
    static std::string GenerateSalt();

    static UserRegistration userRegistration;
    static std::unique_ptr<pqxx::connection> c;
};
