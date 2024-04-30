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
    while (true) {
        // Read the size of the incoming data
        uint32_t size;
        int bytesReceived = SSL_read(ssl, reinterpret_cast<char*>(&size), sizeof(uint32_t));
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            std::cerr << "Error in SSL_read(). Error code: " << errorCode << ". Quitting" << std::endl;
            break;
        }
        size = ntohl(size); // Convert from network byte order to host byte order

        // Read the actual data
        char buffer[size];
        bytesReceived = SSL_read(ssl, buffer, size);
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            std::cerr << "Error in SSL_read(). Error code: " << errorCode << ". Quitting" << std::endl;
            break;
        }

        // Pass the binary data to handleMsg
        std::string response = handleMsg(buffer, bytesReceived);

        // Convert the response to binary
        std::string binaryString(response.begin(), response.end());

        // Send the binary data
        SSL_write(ssl, binaryString.c_str(), binaryString.size());
        if (response == "c" || response == "E") {
            std::cout << "Closing connection with client " << username << std::endl;
            return;
        }
    }
}

std::string ClientHandler::handleMsg(const char* receivedData, int dataSize) {
    std::string response="E";

    // Extract the identifier from the received data
    unsigned char identifier = receivedData[0]; // The identifier is the first byte

    // The actual data starts after the identifier
    const char* dataStart = receivedData + 1;
    int dataLength = dataSize - 1;

    if (this->username.empty() && identifier != 'r' && identifier != 'l') {
        std::cerr << "Client is not logged in. Closing connection." << std::endl;
        return "c";
    }

    if (receivedData[0] == 's') {
        if (dataLength != sizeof(glm::vec3) * 16) {
            std::cerr << "Invalid data size for identifier 's'. Expected " << sizeof(glm::vec3) * 16 << " bytes, got " << dataLength << " bytes." << std::endl;
            return "E";
        }

        // Convert the binary data back to glm::vec3
        glm::vec3 v1[8];
        glm::vec3 v2[8];
        memcpy(v1, receivedData + 1, sizeof(glm::vec3) * 8);
        memcpy(v2, receivedData + 1 + sizeof(glm::vec3) * 8, sizeof(glm::vec3) * 8);

        for (int i = 0; i < 8; ++i) {
            std::cout << "v1[" << i << "] = " << glm::to_string(v1[i]) << std::endl;
            std::cout << "v2[" << i << "] = " << glm::to_string(v2[i]) << std::endl;
        }
        //TODO Handle the glm::vec3 data...
        response = "S";

    } else {
        std::string msg(dataStart, dataLength);

        switch (receivedData[0]) {
            case 'r': {
                std::string username = getNextArg(msg);
                response = registerUser(username, msg);
                break;
            }
            case 'l': {
                std::string username = getNextArg(msg);
                response = loginUser(username, msg);
                break;
            }
            case 'm': {
                std::string reciverUsername = getNextArg(msg);
                response = sendMessage(reciverUsername, msg);
                break;
            }
            case 'g': {
                std::string senderUsername = getNextArg(msg);
                int offset = std::stoi(msg); //! Add error handling if the string is not a number
                std::vector<std::pair<std::string, std::string>> messages = getMessages(senderUsername, offset);
                response = "g";
                for (const auto& pair : messages) {
                    std::string timestamp = pair.second;
                    std::string message = pair.first;
                    response += timestamp + (char)30 + message + (char)30;
                }
                break;
            }
            case 'n': {
                std::string senderUsername = getNextArg(msg);
                std::vector<std::pair<std::string, std::string>> messages = getNewMessages(senderUsername);
                response = "n";
                for (const auto& pair : messages) {
                    std::string timestamp = pair.second;
                    std::string message = pair.first;
                    response += timestamp + (char)30 + message + (char)30;
                }
                break;
            }
            case 'c': {
                response = "c";
                break;
            }
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

#ifdef _WIN32
        ctime_s(buffer, sizeof buffer, &now_c);
#else
        ctime_r(&now_c, buffer);
#endif
        
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
    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos+1); // Erase everything up to and including the next (char)30

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