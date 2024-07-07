#include "ClientServiceLink.hpp"

int ClientServiceLink::sock = 0;
std::vector<std::string> ClientServiceLink::messageBuffer;
std::mutex ClientServiceLink::bufferMutex;


void ClientServiceLink::ConnectToTcpServer(const std::string& ip, int port) {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            throw std::runtime_error("Winsock initialization failed");
        }
    #endif

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Error creating socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Error connecting to server");
    }
}

void ClientServiceLink::DisconnectFromTcpServer() {
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
}

void ClientServiceLink::HandleConnection(int socket) {
    const int bufferSize = 1024;
    char buffer[bufferSize];

    while (true) {
        memset(buffer, 0, bufferSize); // Počisti buffer pred vsakim prejemom
        int bytesReceived = recv(socket, buffer, bufferSize, 0);

        if (bytesReceived == 0) {
            // Povezava je bila zaprta
            break;
        } else if (bytesReceived < 0) {
            // Prišlo je do napake pri prejemanju
            throw std::runtime_error("Error receiving data from server");
        }

        // Zakleni mutex pred dostopom do bufferja
        std::lock_guard<std::mutex> lock(bufferMutex);
        // Shrani prejeto sporočilo v buffer
        messageBuffer.push_back(std::string(buffer, bytesReceived));
    }
}

void ClientServiceLink::SendMessage(int socket, const std::string& message) {
    send(sock, message.c_str(), message.length(), 0);
}
