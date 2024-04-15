#pragma once

#include "ClientHandler.hpp"
#include "Config.hpp"


#include <openssl/ssl.h>
#include <openssl/err.h>


#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


class Server {
public:
    Server(int port, const std::string& dir);
    ~Server();
    int initNetwork();
    int acceptClient();
    void handleClient(int clientSocket);

private:
    std::string dir;
    int port;
    std::vector<std::unique_ptr<ClientHandler>> handlers;
    std::unique_ptr<pqxx::connection> c;

    SSL_CTX* ctx;
    SSL* ssl;

    #ifdef _WIN32
        SOCKET serverSocket;
        SOCKET clientSocket;
    #else
        int serverSocket;
        int clientSocket;
    #endif
};