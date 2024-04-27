#include "ClientHandler.hpp"

ClientHandler::ClientHandler(int clientSocket, SSL* ssl, pqxx::connection& c)
: clientSocket(clientSocket), ssl(ssl), c(c), dbConnectionFailed(false) {
    // Preverjanje povezave z bazo podatkov
    if (!c.is_open()) {
        std::cerr << "Can't open database" << std::endl;
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
    int bufferSize = Config::GetInstance().messageBufferSize;
    char buffer[bufferSize];
    while (true) {
        memset(buffer, 0, bufferSize);
        int bytesReceived = SSL_read(ssl, buffer, bufferSize);
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            std::cerr << "Error in SSL_read(). Error code: " << errorCode << ". Quitting" << std::endl;
            break;
        }

        // Pass the binary data to handleMsg
        std::string response = handleMsg(buffer, bytesReceived);
        // Convert the response to binary
        const char* binaryResponse = reinterpret_cast<const char*>(&response);
        std::string binaryString(binaryResponse, binaryResponse + sizeof(std::string));
        SSL_write(ssl, binaryString.c_str(), binaryString.size());
        if (response == "c" || response == "q") {
            return;
        }
    }
}

std::string ClientHandler::handleMsg(const char* receivedData, int dataSize) {
    std::string response;

    if (receivedData[0] == 's') {
        // Convert the binary data back to glm::vec3
        glm::vec3 v1[8];
        glm::vec3 v2[8];
        memcpy(v1, receivedData + 1, sizeof(glm::vec3) * 8);
        memcpy(v2, receivedData + 1 + sizeof(glm::vec3) * 8, sizeof(glm::vec3) * 8);

        //TODO Handle the glm::vec3 data...

    } else {
        // Convert the binary data to a string
        std::string msg(receivedData, dataSize);

        if (msg[0] == 'r') {
            std::string username = getNextArg(msg);
            std::string password = getNextArg(msg);
            response = registerUser(username, password);

        } else if (msg[0] == 'l') {
            std::string username = getNextArg(msg);
            std::string password = getNextArg(msg);
            response = loginUser(username, password);

        } else if (msg[0] == 'm') {
            std::string reciverUsername = getNextArg(msg);
            msg = getNextArg(msg);
            response = sendMessage(reciverUsername, msg);

        } else if (msg[0] == 'g') {
            std::string senderUsername = getNextArg(msg);
            int offset = std::stoi(getNextArg(msg));
            std::vector<std::pair<std::string, std::string>> messages = getMessages(senderUsername, offset);
            response = "g";
            for (const auto& pair : messages) {
                std::string timestamp = pair.second;
                std::string message = pair.first;
                response += timestamp + (char)30 + message + (char)30;
            }

        } else if (msg[0] == 'n') {
            std::string senderUsername = getNextArg(msg);
            std::vector<std::pair<std::string, std::string>> messages = getNewMessages(senderUsername);
            response = "n";
            for (const auto& pair : messages) {
                std::string timestamp = pair.second;
                std::string message = pair.first;
                response += timestamp + (char)30 + message + (char)30;
            }

        } else if (msg[0] == 'c') {
            response = "c";

        } else {
            response = "q";
        }
    }

    return response;
}

char ClientHandler::registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;

    try {
        pqxx::work W(c);

        std::string salt = generateSalt();
        std::string passwordHash = generateHash(password, salt);

        // Get current date/time
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        
        char buffer[26];
        ctime_s(buffer, sizeof buffer, &now_c);
        std::string creationDate(buffer);

        std::string sql = "INSERT INTO Users (Username, PasswordHash, Salt, CreationDate) VALUES ($1, $2, $3, $4);";
        W.exec_params(sql, username, passwordHash, salt, creationDate);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to register user: " << e.what() << std::endl;
        return 'e';
    }
    this->username = username;
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
                this->username = username;
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

char ClientHandler::sendMessage(const std::string& receiverUsername, const std::string& message) {
    char response = 's';
    try {
        pqxx::work W(c);

        std::string sql = "INSERT INTO Messages (SenderUsername, ReceiverUsername, Message) VALUES ($1, $2, $3);";
        W.exec_params(sql, this->username, receiverUsername, message);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to send message: " << e.what() << std::endl;
        response = 'c';
    }

    return response;
}

std::vector<std::pair<std::string, std::string>> ClientHandler::getMessages(std::string senderUsername, int offset) {
    std::vector<std::pair<std::string, std::string>> messages;
    try {
        pqxx::work W(c);

        // Pridobite ID zadnjega prebranega sporočila, če je offset večji od 0, sicer uporabite 0
        int lastReadMessageId = 0;
        if (offset > 0) {
            std::string sql = "SELECT MAX(ID) FROM Messages WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND IsRead = TRUE;";
            pqxx::result R = W.exec_params(sql, this->username, senderUsername);
            lastReadMessageId = R[0][0].is_null() ? 0 : R[0][0].as<int>();
        }

        // Pridobite sporočila in datume
        std::string sql = "SELECT ID, Message, Timestamp FROM Messages WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND ID > $3 ORDER BY Timestamp DESC LIMIT $4 OFFSET $5;";
        pqxx::result R = W.exec_params(sql, this->username, senderUsername, lastReadMessageId, Config::GetInstance().messageBatchSize, offset);
        
        // Ker so sporočila urejena v obratnem vrstnem redu (najnovejše je prvo), jih obrnemo, da bodo v pravilnem vrstnem redu.
        std::vector<pqxx::row> rows(R.begin(), R.end());
        std::reverse(rows.begin(), rows.end());
        
        for (auto row : rows) {
            std::string message = row[1].as<std::string>();
            std::string timestamp = row[2].as<std::string>();
            messages.push_back(std::make_pair(message, timestamp));
        }

        // Označite nova sporočila kot prebrana
        sql = "UPDATE Messages SET IsRead = TRUE WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND ID > $3;";
        W.exec_params(sql, this->username, senderUsername, lastReadMessageId);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to get messages: " << e.what() << std::endl;
    }

    return messages;
}

std::vector<std::pair<std::string, std::string>> ClientHandler::getNewMessages(std::string senderUsername) {
    std::vector<std::pair<std::string, std::string>> messages;
    try {
        pqxx::work W(c);

        // Pridobite ID zadnjega prebranega sporočila
        std::string sql = "SELECT MAX(ID) FROM Messages WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND IsRead = TRUE;";
        pqxx::result R = W.exec_params(sql, this->username, senderUsername);
        int lastReadMessageId = R[0][0].is_null() ? 0 : R[0][0].as<int>();

        // Pridobite nova sporočila
        sql = "SELECT ID, Message, Timestamp FROM Messages WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND ID > $3 ORDER BY Timestamp DESC LIMIT $4;";
        R = W.exec_params(sql, this->username, senderUsername, lastReadMessageId, Config::GetInstance().messageBatchSize);

        // Ker so sporočila urejena v obratnem vrstnem redu (najnovejše je prvo), jih obrnemo, da bodo v pravilnem vrstnem redu.
        std::vector<pqxx::row> rows(R.begin(), R.end());
        std::reverse(rows.begin(), rows.end());

        for (auto row : rows) {
            std::string message = row[1].as<std::string>();
            std::string timestamp = row[2].as<std::string>();
            messages.push_back(std::make_pair(message, timestamp));
        }

        // Označite nova sporočila kot prebrana
        sql = "UPDATE Messages SET IsRead = TRUE WHERE ReceiverUsername = $1 AND SenderUsername = $2 AND ID > $3;";
        W.exec_params(sql, this->username, senderUsername, lastReadMessageId);
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "Failed to get messages: " << e.what() << std::endl;
    }

    return messages;
}

std::string ClientHandler::getNextArg(std::string& msg) {
    msg.erase(0, 1);

    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos); // Erase everything up to the next (char)30

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