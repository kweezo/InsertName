#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <functional>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "SettingsManager.hpp"


class ClientServiceLink{
public:
    static void StartClient(const std::string& dir);
    static void DisconnectFromTcpServer();

    static void SetMessageHandler(std::function<void(const std::string&)> handler);

    template<typename... Args>
    static void SendData(const Args&... args);

    static std::string GetFirstParameter(std::string& message);
    
private:
    static void ProcessMessages();
    static void ProcessSendBuffer();
    static bool ConnectToTcpServer(const std::string& ip, int port);
    static void HandleConnection();
    static bool SendDataFromBuffer(const std::string& message);

    static void HandleMessageContent(const std::string& message);

    template<typename T>
    static void addToStream(std::stringstream& ss, const T& value);
    template<typename T, typename... Args>
    static void addToStream(std::stringstream& ss, const T& first, const Args&... args);
    template<typename... Args>
    static std::string CreateMessage(const Args&... args);

    static int sock;
    static std::mutex sockMutex;
    static std::vector<std::string> messageBuffer;
    static std::mutex bufferMutex;
    static std::vector<std::string> sendBuffer;
    static std::mutex sendBufferMutex;
    static int serviceId;
    static std::function<void(const std::string&)> messageHandler;
};

// ---------------------------- Template functions ---------------------------- //

template<typename... Args>
void ClientServiceLink::SendData(const Args&... args) {
    std::string msg = CreateMessage(serviceId, args...);
    std::lock_guard<std::mutex> lock(sendBufferMutex);
    sendBuffer.push_back(msg);
}

template<typename T>
void ClientServiceLink::addToStream(std::stringstream& ss, const T& value) {
    ss << value;
}

template<typename T, typename... Args>
void ClientServiceLink::addToStream(std::stringstream& ss, const T& first, const Args&... args) {
    ss << first << static_cast<char>(30);
    addToStream(ss, args...);
}

template<typename... Args>
std::string ClientServiceLink::CreateMessage(const Args&... args) {
    std::stringstream ss;
    addToStream(ss, args...);
    return ss.str();
}
