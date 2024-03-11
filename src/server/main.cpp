#include <iostream>
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

int main() {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Failed to initialize winsock\n";
            return -1;
        }
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #else
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #endif

    if (serverSocket == -1) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Failed to bind socket\n";
        return -1;
    }

    if (listen(serverSocket, 3) == -1) {
        std::cerr << "Failed to listen on socket\n";
        return -1;
    }

    std::cout << "Server is listening on port 12345...\n";

    while (true) {
        sockaddr_in clientAddress{};
        #ifdef _WIN32
            int clientAddressLength = sizeof(clientAddress);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        #else
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        #endif

        if (clientSocket == -1) {
            std::cerr << "Failed to accept client connection\n";
            continue;
        }

        std::cout << "Accepted client connection\n";
        // Handle client connection...
    }

    #ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(serverSocket);
    #endif

    return 0;
}