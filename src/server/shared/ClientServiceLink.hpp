#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <mutex>
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

class ClientServiceLink{
public:
    static void StartClient();
    static void DisconnectFromTcpServer();

    template<typename... Args>
    static void SendMessage(const Args&... args);

private:
    static bool ConnectToTcpServer(const std::string& ip, int port);
    static void HandleConnection();
    static void Send(const std::string& message);

    template<typename T>
    static void addToStream(std::stringstream& ss, const T& value);
    template<typename T, typename... Args>
    static void addToStream(std::stringstream& ss, const T& first, const Args&... args);
    template<typename... Args>
    static std::string CreateMessage(const Args&... args);

    static int sock;
    static std::vector<std::string> messageBuffer;
    static std::mutex bufferMutex;
};

// ---------------------------- Template functions ----------------------------

template<typename... Args>
void ClientServiceLink::SendMessage(const Args&... args) {
    std::string msg = CreateMessage(args...);
    Send(msg);
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