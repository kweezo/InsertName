#include "Auth.hpp"

#include "Settings.hpp"
#include "common/TypeUtils.hpp"

#include <iostream>
#include <stdexcept>

std::atomic<std::shared_ptr<pqxx::connection>> Auth::c;
// std::atomic<std::unordered_map<unsigned, UserRegistration>> serRegistrations;
// std::atomic<unsigned> userRegistrationsIndex;


void Auth::Init() {
    std::string connString = settings.dbConnString;
    c = std::make_unique<pqxx::connection>(settings.dbConnString);
    if (!c.load()->is_open()) {
        throw std::runtime_error("Failed to open database connection");
    }

    pqxx::work txn(*c.load());
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

short Auth::RegisterUser(const std::string& username, const std::string& password, const std::string& email) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        std::string salt;
        do {
            salt = TypeUtils::generateSalt();
        } while (salt == "");
        std::string passwordHash = TypeUtils::hashString(password, salt);
        double createdAt = TypeUtils::getCurrentTimestamp();

        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO users (username, email, createdAt, passwordHash, passwordSalt) VALUES ($1, $2, $3, $4, $5)",
            username, email, createdAt, passwordHash, salt
        );
        txn.commit();

        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error registering user: " << e.what() << std::endl;
        return -9;
    }
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

short Auth::ChangePassword(int uid, const std::string& oldPassword, const std::string& newPassword) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        short verifyResult = VerifyPassword(uid, oldPassword);
        if (verifyResult != 1) {
            return verifyResult;
        }

        std::string newSalt;
        do {
            newSalt = TypeUtils::generateSalt();
        } while (newSalt == "");
        std::string newPasswordHash = TypeUtils::hashString(newPassword, newSalt);

        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE users SET passwordHash = $1, passwordSalt = $2 WHERE uid = $3",
            newPasswordHash, newSalt, uid
        );
        txn.commit();

        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error changing password: " << e.what() << std::endl;
        return -9;
    }
}

short Auth::VerifyPassword(int uid, const std::string& password) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT passwordHash, passwordSalt FROM users WHERE uid = $1",
            uid
        );

        if (result.empty()) { // User not found
            return -1;
        }

        std::string storedHash = result[0]["passwordHash"].as<std::string>();
        std::string storedSalt = result[0]["passwordSalt"].as<std::string>();

        std::string computedHash = TypeUtils::hashString(password, storedSalt);

        if (storedHash == computedHash) return 1;
        else return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error verifying password: " << e.what() << std::endl;
        return -9;
    }
}

bool Auth::VerifyEmail(const std::string& email) {
    //TODO Implement email verification logic
    return true;
}

short Auth::CheckUsername(const std::string& username) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT COUNT(*) FROM users WHERE username = $1",
            username
        );

        if (result[0][0].as<int>() > 0) {
            return 1; // Username exists
        } else {
            return 0; // Username does not exist
        }
    } catch (const std::exception& e) {
        std::cerr << "Error checking username: " << e.what() << std::endl;
        return -9;
    }
}

bool Auth::CheckPassword(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c)) hasSpecial = true;
    }

    if (hasUpper && hasLower && hasDigit && hasSpecial) {
        return true;
    } else {
        return false;
    }
}

bool Auth::CheckEmail(const std::string& email) {
    //TODO Implement email check logic
    return true;
}

std::string Auth::CreateReloginToken(int uid) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return "";
        }

        std::string reloginToken;
        std::string reloginTokenSalt;
        do {
            reloginToken = TypeUtils::generateSalt();
        } while (reloginToken == "");
        do {
            reloginTokenSalt = TypeUtils::generateSalt();
        } while (reloginTokenSalt == "");

        std::string reloginTokenHash = TypeUtils::hashString(reloginToken, reloginTokenSalt);

        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE users SET reloginTokenHash = $1, reloginTokenSalt = $2 WHERE uid = $3",
            reloginTokenHash, reloginTokenSalt, uid
        );
        txn.commit();

        return reloginToken;
    } catch (const std::exception& e) {
        std::cerr << "Error creating relogin token: " << e.what() << std::endl;
        return "";
    }
}

short Auth::VerifyReloginToken(int uid, const std::string& token) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT reloginTokenHash, reloginTokenSalt FROM users WHERE uid = $1",
            uid
        );

        if (result.empty()) { // User not found
            return -1;
        }

        std::string storedHash = result[0]["reloginTokenHash"].as<std::string>();
        std::string storedSalt = result[0]["reloginTokenSalt"].as<std::string>();

        std::string computedHash = TypeUtils::hashString(token, storedSalt);

        if (storedHash == computedHash) return 1;
        else return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error verifying relogin token: " << e.what() << std::endl;
        return -9;
    }
}

int Auth::GetUID(const std::string& username) {
    try {
        auto conn = c.load();
        if (!conn) {
            std::cerr << "Database connection is not open" << std::endl;
            return -10;
        }

        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT uid FROM users WHERE username = $1",
            username
        );

        if (result.empty()) { // User not found
            return -1;
        }

        return result[0]["uid"].as<int>();
    } catch (const std::exception& e) {
        std::cerr << "Error getting UID: " << e.what() << std::endl;
        return -9;
    }
}
