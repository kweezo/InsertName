#pragma once

#include "HandleClient.hpp"

#include <stdexcept>
#include <iostream>


#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


class NetworkManager {

public:
    NetworkManager(int port);
    ~NetworkManager();
    int initNetwork();
    int acceptClient();
    void handleClientConnection();

private:
    int port;

    #ifdef _WIN32
        SOCKET serverSocket;
        SOCKET clientSocket;
    #else
        int serverSocket;
        int clientSocket;
    #endif
};