#include "ClientHandler.hpp"

#include "AdminConsole.hpp"
#include "Log.hpp"


void ClientHandler::handleConnection(pqxx::connection& c, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, fd_set& readfds) {
    int UID = getUID(clientSocket, UIDs, mapMutex);

    std::unique_lock<std::mutex> lock(mapMutex);

    auto it = UIDs.find(clientSocket);
    if (it == UIDs.end()) {
        // Handle error: no such clientSocket in the map
        return;
    }

    lock.unlock();

    SSL* ssl = it->second.second;

    // Read the size of the incoming data
    uint32_t size;
    int bytesReceived;
    do {
        bytesReceived = SSL_read(ssl, reinterpret_cast<char*>(&size), sizeof(uint32_t));
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
                // The operation would block, try again later
                continue;
            } else {
                // An actual error occurred
                Log::print(2, "Error in SSL_read(). Error code: " + std::to_string(errorCode) + ". Quitting");
                cleanupConnection(clientSocket, UIDs, ssl, mapMutex, readfds);
                return;
            }
        }
    } while (bytesReceived <= 0);
    
    size = ntohl(size); // Convert from network byte order to host byte order
    
    // Read the actual data
    char buffer[size];
    do {
        bytesReceived = SSL_read(ssl, buffer, size);
        if (bytesReceived <= 0) {
            int errorCode = SSL_get_error(ssl, bytesReceived);
            if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
                // The operation would block, try again later
                continue;
            } else {
                // An actual error occurred
                Log::print(2, "Error in SSL_read(). Error code: " + std::to_string(errorCode) + ". Quitting");
                cleanupConnection(clientSocket, UIDs, ssl, mapMutex, readfds);
                return;
            }
        }
    } while (bytesReceived <= 0);

    std::string response = handleMsg(buffer, bytesReceived, clientSocket, UIDs, mapMutex, c);

    std::string binaryString(response.begin(), response.end());

    // Send the binary data
    int bytesSent;
    fd_set writefds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Set the timeout to zero for non-blocking operation

    do {
        FD_ZERO(&writefds);
        FD_SET(clientSocket, &writefds);

        // Check if writing is possible
        int selectResult = select(clientSocket + 1, NULL, &writefds, NULL, &timeout);
        if (selectResult < 0) {
            // An error occurred
            Log::print(3, "select error occurred on server socket");
            cleanupConnection(clientSocket, UIDs, ssl, mapMutex, readfds);
            return;
        } else if (selectResult == 0) {
            // Writing would block, try again later
            continue;
        }

        bytesSent = SSL_write(ssl, binaryString.c_str(), binaryString.size());
        if (bytesSent <= 0) {
            int errorCode = SSL_get_error(ssl, bytesSent);
            if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
                // The operation would block, try again later
                continue;
            } else {
                // An actual error occurred
                Log::print(3, "Error in SSL_write(). Error code: " + std::to_string(errorCode) + ". Quitting");
                cleanupConnection(clientSocket, UIDs, ssl, mapMutex, readfds);
                return;
            }
        }
    } while (bytesSent <= 0);

    if (response == "c" || response == "E") {
        if (response == "E") Log::print(2, "Closing connection with client becuse error occured, UID: " + std::to_string(UID));
        cleanupConnection(clientSocket, UIDs, ssl, mapMutex, readfds);
        return;
    }
}

std::string ClientHandler::handleMsg(const char* receivedData, int dataSize, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c) {
    int UID = getUID(clientSocket, UIDs, mapMutex);
    std::string response="E";

    // Extract the identifier from the received data
    unsigned char identifier = receivedData[0]; // The identifier is the first byte

    // The actual data starts after the identifier
    const char* dataStart = receivedData + 1;
    int dataLength = dataSize - 1;

    #ifndef NO_DB
        if (UID <= 0) {
            auto it = UIDs.find(clientSocket);
            if (it != UIDs.end()) {
                --it->second.first;
            }
            UID--;
            if ((identifier != 'r' && identifier != 'l')) {
                return "c";
            }
        }
    #endif

    if (receivedData[0] == 's') {
        if (dataLength != sizeof(glm::vec3) * 16) {
            Log::print(2, "Invalid data size for identifier 's'. Expected " + std::to_string(sizeof(glm::vec3) * 16) + " bytes, got " + std::to_string(dataLength) + " bytes. UID: " + std::to_string(UID));
            return "E";
        }

        // Convert the binary data back to glm::vec3
        glm::vec3 v1[8];
        glm::vec3 v2[8];
        memcpy(v1, receivedData + 1, sizeof(glm::vec3) * 8);
        memcpy(v2, receivedData + 1 + sizeof(glm::vec3) * 8, sizeof(glm::vec3) * 8);

        //TODO Handle the glm::vec3 data...
        response = "S";

    } else {
        std::string msg(dataStart, dataLength);

        switch (receivedData[0]) {
            case 'r': {
                std::string username = getNextArg(msg);
                response = registerUser(username, msg, clientSocket, UIDs, mapMutex, c);
                break;
            }
            case 'l': {
                std::string username = getNextArg(msg);
                response = loginUser(username, msg, clientSocket, UIDs, mapMutex, c);
                break;
            }
/*            case 'm': {
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
            }*/
            case 'c': {
                response = "c";
                break;
            }
        }
    }
    #ifndef NO_DB
        if (UID < -Config::loginAttempts && response != "r" && response != "l") {
            response = "c";
        }
    #endif

    return response;
}

char ClientHandler::registerUser(const std::string& username, const std::string& password, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c) {
    try {
        #ifndef NO_DB
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

            std::string sql = "INSERT INTO Users (UID, Username, PasswordHash, Salt, CreationDate) VALUES (DEFAULT, $1, $2, $3, $4) RETURNING UID;";
            pqxx::result R = W.exec_params(sql, username, passwordHash, salt, creationDate);
            W.commit();
            int UID = R[0][0].as<int>();
        #else
            int UID = clientSocket;
        #endif

        std::lock_guard<std::mutex> lock(mapMutex);
        SSL* ssl = UIDs[clientSocket].second;
        UIDs[clientSocket] = std::make_pair(UID, ssl);

        Log::print(0, "Logging in user with UID: " + std::to_string(UID));

    } catch (const std::exception &e) {
        Log::print(1, "Failed to register user: " + std::string(e.what()));
        return 'e';
    }

    return 'r';
}

char ClientHandler::loginUser(const std::string& username, const std::string& password, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c) {

    try {
        #ifndef NO_DB
            pqxx::work W(c);

            std::string sql = "SELECT UID, PasswordHash, Salt FROM Users WHERE Username = $1;";
            pqxx::result R = W.exec_params(sql, username);

            if (!R.empty()) {
                int UID = R[0][0].as<int>();
                std::string storedPasswordHash = R[0][1].as<std::string>();
                std::string salt = R[0][2].as<std::string>();
                std::string passwordHash = generateHash(password, salt);

                if (storedPasswordHash == passwordHash) {
                    std::lock_guard<std::mutex> lock(mapMutex);
                    SSL* ssl = UIDs[clientSocket].second;
                    UIDs[clientSocket] = std::make_pair(UID, ssl);
                    Log::print(0, "Logging in user with UID: " + std::to_string(getUID(clientSocket, UIDs, mapMutex)));
                    return 'l';
                } else {
                    return 'w';
                }
            } else {
                return 'w';
            }

        #else
            std::lock_guard<std::mutex> lock(mapMutex);
            SSL* ssl = UIDs[clientSocket].second;
            UIDs[clientSocket] = std::make_pair(clientSocket, ssl);
            return 'l';
            Log::print(0, "Logging in user with UID: " + std::to_string(getUID(clientSocket, UIDs, mapMutex)));
        #endif


    } catch (const std::exception &e) {
        Log::print(1, "Failed to login user: " + std::string(e.what()));
        return 'f';
    }
    return 'f';
}
/*
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
        pqxx::result R = W.exec_params(sql, this->username, senderUsername, lastReadMessageId, Config::messageBatchSize, offset);
        
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
        R = W.exec_params(sql, this->username, senderUsername, lastReadMessageId, Config::messageBatchSize);

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
*/
void ClientHandler::cleanupConnection(int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, SSL* ssl, std::mutex& mapMutex, fd_set& readfds) {
    Log::print(0, "Client disconnected with UID: " + std::to_string(getUID(clientSocket, UIDs, mapMutex)));

    // Lock the mutex before accessing the shared map
    std::lock_guard<std::mutex> lock(mapMutex);

    // Remove the client from the map
    UIDs.erase(clientSocket);

    // Remove the client's socket descriptor from the readfds set
    FD_CLR(clientSocket, &readfds);

    // Properly clean up the SSL object
    SSL_shutdown(ssl);
    SSL_free(ssl);

    #ifdef _WIN32
        closesocket(clientSocket);
    #else
        close(clientSocket);
    #endif
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

int ClientHandler::getUID(int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex) {
    std::lock_guard<std::mutex> lock(mapMutex);
    auto it = UIDs.find(clientSocket);
    if (it != UIDs.end()) {
        return it->second.first; // Return the users's ID
    } else {
        return 0;
    }
}
