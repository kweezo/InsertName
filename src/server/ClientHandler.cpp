#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientSocket, SSL* ssl, pqxx::connection& c)
: clientSocket(clientSocket), ssl(ssl), c(c), dbConnectionFailed(false) {
    // Preverjanje povezave z bazo podatkov
    if (c.is_open()) {
        std::cout << "Opened database successfully: " << c.dbname() << std::endl;
    } else {
        std::cerr << "Can't open database" << std::endl;
        dbConnectionFailed = true;
        return;
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

    try {
        pqxx::work W(c);

        std::string salt = generateSalt();
        std::string passwordHash = generateHash(password, salt);

        std::string sql = "INSERT INTO Users (Username, PasswordHash, Salt) VALUES ($1, $2, $3);";
        W.exec_params(sql, username, passwordHash, salt);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to register user: " << e.what() << std::endl;
        return 'e';
    }

    return 'r';
}

char ClientHandler::loginUser(const std::string& username, const std::string& password) {
    std::cout << "Logging in user: " << username << std::endl;

    try {
        pqxx::work W(c);

        std::string sql = "SELECT PasswordHash, Salt FROM Users WHERE Username = $1;";
        pqxx::result R = W.exec_params(sql, username);

        if (!R.empty()) {
            std::string storedPasswordHash = R[0][0].as<std::string>();
            std::string salt = R[0][1].as<std::string>();
            std::string passwordHash = generateHash(password, salt);

            if (storedPasswordHash == passwordHash) {
                return 'l';
            } else {
                return 'w';
            }
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

std::string ClientHandler::generateSalt() {
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));

    std::stringstream ss;
    for(int i = 0; i < sizeof(salt); i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];

    return ss.str();
}

std::string ClientHandler::generateHash(const std::string& password, const std::string& salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    std::string saltedPassword = password + salt;

    SHA256((unsigned char*)saltedPassword.c_str(), saltedPassword.size(), hash);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}