#include "Auth.hpp"


pqxx::connection* Auth::conn = nullptr;

bool Auth::Init() {
    conn = new pqxx::connection("dbname=postgres user=postgres password=postgres hostaddr=127.0.0.1 port=5432");

    return true;
}

int Auth::RegisterUser(const std::string& username, const std::string& password, const std::string& email) {
    try {
        pqxx::work W(*conn);

        std::string sql = "SELECT * FROM Users WHERE Username = $1 OR Email = $2;";
        pqxx::result R = W.exec_params(sql, username, email);
        if (!R.empty()) {
            return 1;
        }

        std::string salt = GenerateSalt();
        std::string passwordHash = HashPassword(password, salt);

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();

        // Convert to total microseconds
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        // Calculate days and fractional day with maximum precision
        double days = static_cast<double>(microseconds) / 86400000000.0; // Total days with maximum precision
        double creationDate = days;

        sql = "INSERT INTO Users (UID, Username, PasswordHash, Salt, CreationDate) VALUES (DEFAULT, $1, $2, $3, $4) RETURNING UID;";
        R = W.exec_params(sql, username, passwordHash, salt, creationDate);
        W.commit();

        return 0;

    } catch (const std::exception &e) {
        return -1;
    }

    return -1;
}

bool Auth::LoginUser(const std::string& username, const std::string& password) {
    return true;
}

std::string Auth::HashPassword(const std::string& password, const std::string& salt) {
    return password;
}

std::string Auth::GenerateSalt() {
    return "salt";
}
