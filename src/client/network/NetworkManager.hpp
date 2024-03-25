#pragma once

#include <string>
#include <iostream>

#ifdef _WIN32
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif


class NetworkManager {
public:
    NetworkManager(const std::string& serverIP, int port);
    ~NetworkManager();
    bool connectToServer();
    bool sendAndReceiveData();

private:
    std::string serverIP;
    int port;
    int sock;
};