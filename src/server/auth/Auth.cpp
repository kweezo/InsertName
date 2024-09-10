#include "Auth.hpp"

#include "Settings.hpp"

#include <stdexcept>

UserRegistration Auth::userRegistration;
std::unique_ptr<pqxx::connection> Auth::c;


void Auth::Init() {
    std::string connString = settings.dbConnString;
    c = std::make_unique<pqxx::connection>(settings.dbConnString);
    if (!c->is_open()) {
        throw std::runtime_error("Failed to open database connection");
    }

    pqxx::work txn(*c);
    txn.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "uid SERIAL PRIMARY KEY, "
        "username VARCHAR(255) NOT NULL, "
        "email VARCHAR(255) NOT NULL, "
        "createdAt DOUBLE PRECISION NOT NULL"
        "passwordHash VARCHAR(255) NOT NULL, "
        "passwordSalt VARCHAR(255) NOT NULL, "
        "reloginTokenHash VARCHAR(255), "
        "reloginTokenSalt VARCHAR(255)"
        ");"
    );
    txn.commit();
}

bool Auth::RegisterUser(const std::string& username, const std::string& password, const std::string& email) {
    //TODO Implement registration logic
    return true;
}

bool Auth::LoginUser(int uid, const std::string& password) {
    //TODO Implement login logic
    return true;
}

bool Auth::LoginUser(int uid) {
    //TODO Implement login logic
    return true;
}

void Auth::LogoutUser(int uid) {
    //TODO Implement logout logic
}

bool Auth::ChangePassword(int uid, const std::string& oldPassword, const std::string& newPassword) {
    //TODO Implement password change logic
    return true;
}

bool Auth::VerifyPassword(int uid, const std::string& password) {
    //TODO Implement password verification logic
    return true;
}

bool Auth::VerifyEmail(const std::string& email) {
    //TODO Implement email verification logic
    return true;
}

bool Auth::CheckUsername(const std::string& username) {
    //TODO Implement username check logic
    return true;
}

bool Auth::CheckPassword(const std::string& password) {
    //TODO Implement password check logic
    return true;
}

bool Auth::CheckEmail(const std::string& email) {
    //TODO Implement email check logic
    return true;
}

bool Auth::SendReloginToken(int uid) {
    //TODO Implement relogin token sending logic
    return true;
}

bool Auth::VerifyReloginToken(int uid, const std::string& token) {
    //TODO Implement relogin token verification logic
    return true;
}

bool Auth::SendLoginToken(int uid) {
    //TODO Implement login token sending logic
    return true;
}

int Auth::GetUID(const std::string& username) {
    //TODO Implement UID retrieval logic
    return -1;
}

std::string Auth::HashString(const std::string& string, const std::string& salt) {
    //TODO Implement password hashing logic
    return "";
}

std::string Auth::GenerateSalt() {
    //TODO Implement salt generation logic
    return "";
}
