#pragma once

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>

class ClientHandler {
public:
    ClientHandler(int clientSocket, SSL* ssl, pqxx::connection& c);
    ~ClientHandler();

    void handleConnection();
    std::string handleMsg(const std::string& receivedMsg);
    char registerUser(const std::string& username, const std::string& password);
    char loginUser(const std::string& username, const std::string& password);
    std::string getNextArg(std::string& msg);


    std::string generateSalt();
    std::string generateHash(const std::string& password, const std::string& salt);


private:
    int clientSocket;
    SSL* ssl;
    bool dbConnectionFailed;
    pqxx::connection& c;
    std::string username;
};