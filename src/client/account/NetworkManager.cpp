#include "NetworkManager.hpp"



NetworkManager::NetworkManager(const std::string& serverIP, int port)
    : serverIP(serverIP), port(port), sock(INVALID_SOCKET) {
    // Initialize WinSock
    #ifdef _WIN32
        WSAData data;
        WORD ver = MAKEWORD(2, 2);
        int wsResult = WSAStartup(ver, &data);
        if (wsResult != 0) {
            std::cerr << "Can't start Winsock, Err #" << wsResult << '\n';
        }
    #endif
}

NetworkManager::~NetworkManager() {
    // Gracefully close down everything
    closesocket(sock);
    #ifdef _WIN32
        WSACleanup();
    #endif
}

bool NetworkManager::connectToServer() {
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        #ifdef _WIN32
            std::cerr << "Can't create socket, Err #" << WSAGetLastError() << '\n';
        #else
            std::cerr << "Can't create socket, Err #" << errno << '\n';
        #endif
        return false;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &(hint.sin_addr));

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        #ifdef _WIN32
            std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << '\n';
        #else
            std::cerr << "Can't connect to server, Err #" << errno << '\n';
        #endif
        return false;
    } else {
        std::cout << "Connected to server\n";
    }

    return true;
}

std::string NetworkManager::sendData(const std::string& message) {
    // Send the message
    int sendResult = send(sock, message.c_str(), message.size() + 1, 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Failed to send message\n";
        return "";
    }

    // Receive the response
    char buf[1024];
    memset(buf, 0, 1024);
    int bytesReceived = recv(sock, buf, 1024, 0);
    if (bytesReceived > 0) {
        std::string response(buf, 0, bytesReceived);
        std::cout << "SERVER> " << response << '\n';
        return response;
    }
    
    return "";
}