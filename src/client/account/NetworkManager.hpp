#pragma once

#include <string>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <string.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include "Settings.hpp"


class NetworkManager {
public:
    NetworkManager(const std::string& serverIP, int port);
    bool connectToServer();
    void closeConnection();
    //std::string sendData(const std::string& message);
    template <typename T>
    std::string sendData(unsigned char identifier, const T& data);
    struct EmptyStruct {};

private:
    std::string serverIP;
    int port;
    SOCKET sock;
    SSL_CTX* ctx;
    SSL* ssl;
};