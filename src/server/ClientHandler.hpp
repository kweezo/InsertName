#pragma once

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <chrono>

#include <pqxx/pqxx>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <glm/glm.hpp>

#include "Config.hpp"

class ClientHandler {
public:
    ClientHandler(int clientSocket, SSL* ssl, pqxx::connection& c);
    ~ClientHandler();

    void handleConnection();
    std::string handleMsg(const char* receivedData, int dataSize);

    char registerUser(const std::string& username, const std::string& password);
    char loginUser(const std::string& username, const std::string& password);
    char sendMessage(const std::string& receiverUsername, const std::string& message);
    std::vector<std::pair<std::string, std::string>> getMessages(std::string senderUsername, int offset = 0);
    std::vector<std::pair<std::string, std::string>> getNewMessages(std::string senderUsername);

    std::string getNextArg(std::string& msg);
    std::string generateSalt();
    std::string generateHash(const std::string& password, const std::string& salt);


private:
    int clientSocket;
    SSL* ssl;
    bool dbConnectionFailed;
    pqxx::connection& c;

    std::string username;
    int msgBatchSize;
};