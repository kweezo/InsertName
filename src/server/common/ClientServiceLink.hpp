#pragma once

#include <string>
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
    static void ConnectToTcpServer(const std::string& ip, int port);
    static void DisconnectFromTcpServer();

    static void HandleConnection(int socket);
    static void SendMessage(int socket, const std::string& message);

private:
    static int sock;

    static std::vector<std::string> messageBuffer;
    static std::mutex bufferMutex;
};