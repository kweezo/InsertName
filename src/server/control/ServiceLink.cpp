#include "ServiceLink.hpp"

std::mutex ServiceLink::connectionMutex;
std::condition_variable ServiceLink::connectionCond;
int ServiceLink::activeConnections = 0;
const int ServiceLink::maxConnections = 4;
std::vector<Message> ServiceLink::messageBuffer;
std::mutex ServiceLink::bufferMutex;


void ServiceLink::SendMessage(int socket, const std::string& message) {
    send(socket, message.c_str(), message.length(), 0);
}

void ServiceLink::HandleConnection(int socket) {
    char buffer[1024];

    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        activeConnections++;
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        {
            std::lock_guard<std::mutex> bufferLock(bufferMutex);
            messageBuffer.push_back({socket, std::string(buffer, bytesReceived)});
        }
    }

    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        activeConnections--;
    }

    connectionCond.notify_one();

    #ifdef _WIN32
        closesocket(socket);
    #else
        close(socket);
    #endif
}

void ServiceLink::StartTcpServer(int port) {
    #ifdef _WIN32
        WSADATA wsaData;
        SOCKET server_fd, new_socket;
    #else
        int server_fd, new_socket;
    #endif
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    #ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed.\n";
            exit(EXIT_FAILURE);
        }
    #endif

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "ServiceLink server is listening on port " << port << std::endl;

    while (true) {
        std::unique_lock<std::mutex> lock(connectionMutex);
        connectionCond.wait(lock, []{ return activeConnections < maxConnections; });

        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "Connection accepted" << std::endl;
        std::thread t(HandleConnection, new_socket);
        t.detach();
    }

    #ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
    #else
        close(server_fd);
    #endif
}
