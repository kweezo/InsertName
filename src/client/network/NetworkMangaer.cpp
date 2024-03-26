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
    if (sock == INVALID_SOCKET) {
        std::cerr << "Can't create socket, Err #" << WSAGetLastError() << '\n';
        return false;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &(hint.sin_addr));

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << '\n';
        return false;
    }

    return true;
}

bool NetworkManager::sendAndReceiveData() {
    // Do-while loop to send and receive data
    char buf[4096];
    std::string userInput;

    do {
        // Prompt the user for some text
        std::cout << "> ";
        std::getline(std::cin, userInput);

        if (userInput.size() > 0) {
            // Send the text
            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult != SOCKET_ERROR) {
                // Wait for response
                memset(buf, 0, 4096);
                int bytesReceived = recv(sock, buf, 4096, 0);
                if (bytesReceived > 0) {
                    // Echo response to console
                    std::cout << "SERVER> " << std::string(buf, 0, bytesReceived) << '\n';
                }
            }
        }

    } while (userInput.size() > 0);

    return true;
}