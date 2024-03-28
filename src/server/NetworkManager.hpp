#pragma once

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
    NetworkManager();
    ~NetworkManager();
    int initNetwork();
    int acceptClient();
    void handleClientConnection();

    std::string handleMsg(const std::string& receivedMsg);

private:
    #ifdef _WIN32
        SOCKET serverSocket;
        SOCKET clientSocket;
    #else
        int serverSocket;
        int clientSocket;
    #endif
};