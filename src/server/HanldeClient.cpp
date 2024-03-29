#include "HandleClient.hpp"


std::string HandleClient::dir = "";

HandleClient::HandleClient(const std::string& directory) {
    dir = directory;

    // Create the database
    int rc = sqlite3_open(dir.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Create the table
    std::string sql = "CREATE TABLE IF NOT EXISTS Users ("
                      "Username TEXT PRIMARY KEY NOT NULL,"
                      "PasswordHash TEXT NOT NULL);";
    char *errMsg = 0;
    rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg);  // Removed 'int' here

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table created successfully" << std::endl;
    }
}

HandleClient::~HandleClient() {
    sqlite3_close(db);
}

std::string HandleClient::handleMsg(const std::string& receivedMsg) {
    std::string response = "";
    std::string msg = receivedMsg;

    if (receivedMsg[0] == 'r') {
        username = getNextArg(msg);
        std::string password = getNextArg(msg);
        response = registerUser(username, password);
    } else if (receivedMsg[0] == 'l') {
        username = getNextArg(msg);
        std::string password = getNextArg(msg);
        response = loginUser(username, password);
    } else {
        response = "e";
    }

    return response;
}

char HandleClient::registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;

    std::string sql = "INSERT INTO Users (Username, PasswordHash) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to register user: " << sqlite3_errmsg(db) << std::endl;
        return 'e';
    }

    sqlite3_finalize(stmt);
    return 'r';
}

char HandleClient::loginUser(const std::string& username, const std::string& password) {
    std::cout << "Logging in user: " << username << std::endl;

    std::string sql = "SELECT PasswordHash FROM Users WHERE Username = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (storedPassword == password) {
            sqlite3_finalize(stmt);
            return 'l';
        } else {
            sqlite3_finalize(stmt);
            return 'w';
        }
    } else {
        std::cerr << "Failed to login user: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return 'f';
    }
}

std::string HandleClient::getNextArg(std::string& msg) {
    msg.erase(0, 1);

    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos); // Erase everything up to and including the next (char)30

    return arg;
}