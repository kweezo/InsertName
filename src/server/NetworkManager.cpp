#include "NetworkManager.hpp"


NetworkManager::NetworkManager() {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize winsock");
        }
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #else
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #endif

    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }
}

NetworkManager::~NetworkManager() {
    #ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(serverSocket);
    #endif
}

int NetworkManager::initNetwork() {
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 3) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Server is listening on port " << ntohs(serverAddress.sin_port) << "...\n";
    return 0;
}

int NetworkManager::acceptClient() {
    sockaddr_in clientAddress{};
    #ifdef _WIN32
        int clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    #else
        socklen_t clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    #endif

    if (clientSocket == -1) {
        throw std::runtime_error("Failed to accept client connection");
    }

    std::cout << "Accepted client connection\n";
    return 0;
}

void NetworkManager::handleClientConnection() {
    //TODO: Implement this function
}