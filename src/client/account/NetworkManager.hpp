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

template <typename T>
std::string NetworkManager::sendData(unsigned char identifier, const T& data) {
    // Convert the data to binary
    const char* binaryData = reinterpret_cast<const char*>(&data);
    std::string binaryString(binaryData, binaryData + sizeof(T));

    // Prepend the identifier
    binaryString.insert(binaryString.begin(), identifier);

    // Send the binary data
    int sendResult = SSL_write(ssl, binaryString.c_str(), binaryString.size());
    if (sendResult <= 0) {
        // handle error
        return "";
    }

    // Receive the response
    int bufferSize = Settings::GetInstance().messageBufferSize;
    char buf[bufferSize];
    memset(buf, 0, bufferSize);
    int bytesReceived = SSL_read(ssl, buf, bufferSize);
    if (bytesReceived > 0) {
        std::string response(buf, 0, bytesReceived);
        return response;
    }
    
    return "";
}