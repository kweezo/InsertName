#pragma once

#include "Config.hpp"
#include "buildConfig.hpp"
#include "ClientHandler.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <fcntl.h>
#endif

#include <mutex>
#include <unordered_map>


class Server {
public:
    Server(int port, const std::string& dir);
    ~Server();
    int initNetwork();
    int acceptClient();
    void handleClients();

private:
    std::unique_ptr<pqxx::connection> c;
    std::unordered_map<int, std::pair<int, SSL*>> UIDs;
    std::mutex mapMutex;
    std::unordered_map<int, std::mutex> clientMutexes;

    ClientHandler clientHandler;
    
    std::string dir;
    int port;

    SSL_CTX* ctx;

    #ifdef _WIN32
        SOCKET serverSocket;
        SOCKET clientSocket;
    #else
        int serverSocket;
        int clientSocket;
    #endif
};