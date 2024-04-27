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
    std::string binaryString;
    if constexpr (std::is_same_v<T, std::string>) {
        binaryString = data; // No need to convert if T is already a string
    } else {
        const char* binaryData = reinterpret_cast<const char*>(&data);
        binaryString.assign(binaryData, binaryData + sizeof(T));
    }

    // Convert the size to binary
    uint32_t size = htonl(binaryString.size() + 1); // Add 1 for the identifier
    const char* binarySize = reinterpret_cast<const char*>(&size);

    // Insert the identifier at the beginning of the binary string
    binaryString.insert(binaryString.begin(), identifier);

    // Prepend the size to the binary string
    binaryString.insert(binaryString.begin(), binarySize, binarySize + sizeof(uint32_t));

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
        // Convert the binary response back to a string
        std::string response(buf, bytesReceived);
        return response;
    }
    
    return "";
}
