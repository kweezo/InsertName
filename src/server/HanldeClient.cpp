#include "HandleClient.hpp"

std::string HandleClient::dir = "";

HandleClient::HandleClient(const std::string& directory) {
    dir = directory;

    // Create the database connection
    try {
        pqxx::connection c{"dbname=posgres user=postgres password=password hostaddr=127.0.0.1 port=5432"};
        if (c.is_open()) {
            std::cout << "Opened database successfully: " << c.dbname() << std::endl;
        } else {
            std::cerr << "Can't open database" << std::endl;
            return;
        }

        // Create the table
        pqxx::work W(c);
        std::string sql = "CREATE TABLE IF NOT EXISTS Users ("
                          "Username TEXT PRIMARY KEY NOT NULL,"
                          "PasswordHash TEXT NOT NULL);";
        W.exec(sql);
        W.commit();
        std::cout << "Table created successfully" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return;
    }
}

HandleClient::~HandleClient() {
    // PostgreSQL connection is closed automatically when the pqxx::connection object is destroyed
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

    try {
        pqxx::work W(c);

        std::string sql = "INSERT INTO Users (Username, PasswordHash) VALUES ($1, $2);";
        W.exec_params(sql, username, password);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to register user: " << e.what() << std::endl;
        return 'e';
    }

    return 'r';
}

char HandleClient::loginUser(const std::string& username, const std::string& password) {
    std::cout << "Logging in user: " << username << std::endl;

    try {
        pqxx::work W(c);

        std::string sql = "SELECT PasswordHash FROM Users WHERE Username = $1;";
        pqxx::result R = W.exec_params(sql, username);

        if (!R.empty() && R[0][0].as<std::string>() == password) {
            return 'l';
        } else {
            return 'w';
        }
    } catch (const std::exception &e) {
        std::cerr << "Failed to login user: " << e.what() << std::endl;
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