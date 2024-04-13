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

class ClientHandler {
public:
    ClientHandler(int clientSocket, SSL* ssl);
    ~ClientHandler();

    void handleConnection();
    std::string handleMsg(const std::string& receivedMsg);
    char registerUser(const std::string& username, const std::string& password);
    char loginUser(const std::string& username, const std::string& password);
    std::string getNextArg(std::string& msg);


private:
    int clientSocket;
    SSL* ssl;
    bool dbConnectionFailed;
    std::unique_ptr<pqxx::connection> c;
    std::string username;
};