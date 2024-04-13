#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientSocket, SSL* ssl) : clientSocket(clientSocket), ssl(ssl), dbConnectionFailed(false) {
    try {
        c = std::make_unique<pqxx::connection>("dbname=postgres user=postgres password=password hostaddr=127.0.0.1 port=5432");
        if (c->is_open()) {
            std::cout << "Opened database successfully: " << c->dbname() << std::endl;
        } else {
            std::cerr << "Can't open database" << std::endl;
            dbConnectionFailed = true;
            return;
        }

        // Create the table
        pqxx::work W(*c);
        std::string sql = "CREATE TABLE IF NOT EXISTS Users ("
                          "Username TEXT PRIMARY KEY NOT NULL,"
                          "PasswordHash TEXT NOT NULL);";
        W.exec(sql);
        W.commit();
        std::cout << "Table created/openned successfully" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        dbConnectionFailed = true;
    }
}

ClientHandler::~ClientHandler() {
    #ifdef _WIN32
        closesocket(clientSocket);
    #else
        close(clientSocket);
    #endif
}

void ClientHandler::handleConnection() {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytesReceived = SSL_read(ssl, buffer, 4096);
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            std::cerr << "Error in SSL_read(). Error code: " << errorCode << ". Quitting" << std::endl;
            break;
        }

        std::string receivedMsg(buffer);
        std::string response = handleMsg(receivedMsg);
        SSL_write(ssl, response.c_str(), response.size() + 1);
    }
}

std::string ClientHandler::handleMsg(const std::string& receivedMsg) {
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

char ClientHandler::registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;

    if (dbConnectionFailed || !c) {
        std::cerr << "Database connection failed. Cannot register user." << std::endl;
        return 'e';
    }

    try {
        pqxx::work W(*c);

        std::string sql = "INSERT INTO Users (Username, PasswordHash) VALUES ($1, $2);";
        W.exec_params(sql, username, password);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to register user: " << e.what() << std::endl;
        return 'e';
    }

    return 'r';
}

char ClientHandler::loginUser(const std::string& username, const std::string& password) {
    std::cout << "Logging in user: " << username << std::endl;

    if (dbConnectionFailed || !c) {
        std::cerr << "Database connection failed. Cannot login user." << std::endl;
        return 'f';
    }

    try {
        pqxx::work W(*c);

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

std::string ClientHandler::getNextArg(std::string& msg) {
    msg.erase(0, 1);

    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos); // Erase everything up to and including the next (char)30

    return arg;
}