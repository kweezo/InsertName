#pragma once

#include <pqxx/pqxx>

#include <string>
#include <atomic>
#include <unordered_map>


struct UserRegistration {
    std::string username = "";
    std::string password = "";
    std::string email = "";
};

class Auth {
public:
    static void Init();

    static short RegisterUser(const std::string& username, const std::string& password, const std::string& email);
    static bool LoginUser(int uid, const std::string& password);
    static bool LoginUser(int uid);
    static void LogoutUser(int uid);
    static short ChangePassword(int uid, const std::string& oldPassword, const std::string& newPassword);

    static short VerifyPassword(int uid, const std::string& password);
    static bool VerifyEmail(const std::string& email);
    static short CheckUsername(const std::string& username);
    static bool CheckPassword(const std::string& password);
    static bool CheckEmail(const std::string& email);

    static std::string CreateReloginToken(int uid);
    static short VerifyReloginToken(int uid, const std::string& token);

    static int GetUID(const std::string& username);

private:
    static std::atomic<std::shared_ptr<pqxx::connection>> c;

    // static std::atomic<std::unordered_map<unsigned, UserRegistration>> userRegistrations;
    // static std::atomic<unsigned> userRegistrationsIndex;
};

// ----------------------------------------------------------------------------------------

/* Error codes:
    * 1: No error
    * 0: Wrong password
    * -1: User not found
    * -9: Catch-all error
    * -10: Database connection is not open
*/
