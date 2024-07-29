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
#include <map>


class Server {
public:
    static void init(int port, const std::string& dir);
    static void stop();
    static int initNetwork();
    static int acceptClient();
    static void handleClients();

    static bool isShuttingDown;
    static bool shutdown;

private:
    static std::unique_ptr<pqxx::connection> c;
    static std::map<int, std::pair<int, SSL*>> UIDs;
    static std::mutex mapMutex;
    static std::map<int, std::mutex> clientMutexes;

    static ClientHandler clientHandler;
    
    static std::string dir;
    static int port;

    static SSL_CTX* ctx;

    #ifdef _WIN32
        static SOCKET serverSocket;
        static SOCKET clientSocket;
    #else
        static int serverSocket;
        static int clientSocket;
    #endif
};