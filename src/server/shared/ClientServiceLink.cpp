#include "ClientServiceLink.hpp"

int ClientServiceLink::sock = 0;
std::mutex ClientServiceLink::sockMutex;
std::vector<std::string> ClientServiceLink::messageBuffer;
std::mutex ClientServiceLink::bufferMutex;
std::vector<std::string> ClientServiceLink::sendBuffer;
std::mutex ClientServiceLink::sendBufferMutex;
int ClientServiceLink::serviceId = 0;


bool ClientServiceLink::ConnectToTcpServer(const std::string& ip, int port) {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            throw std::runtime_error("Winsock initialization failed");
        }
    #endif

    {
        std::lock_guard<std::mutex> lock(sockMutex);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Error creating socket");
        }
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);

    std::lock_guard<std::mutex> lock(sockMutex);
    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Connection failed\n";
        return false;
    }

    return true;
}

void ClientServiceLink::DisconnectFromTcpServer() {
    std::lock_guard<std::mutex> lock(sockMutex);
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
}

void ClientServiceLink::HandleConnection() {
    const int bufferSize = 1024;
    char buffer[bufferSize];

    while (true) {
        memset(buffer, 0, bufferSize);
        int bytesReceived;
        {
            std::lock_guard<std::mutex> lock(sockMutex);
            bytesReceived = recv(sock, buffer, bufferSize, 0);
        }

        if (bytesReceived <= 0) {
            std::cerr << "Error receiving message from server\n";
            return;
        }

        std::lock_guard<std::mutex> lock(bufferMutex);
        messageBuffer.push_back(std::string(buffer, bytesReceived));
    }
}

void ClientServiceLink::ProcessSendBuffer() {
    while (true) {
        std::unique_lock<std::mutex> bufferLock(sendBufferMutex);
        std::unique_lock<std::mutex> sockLock(sockMutex);
        if (!sendBuffer.empty() && sock > 0) {
            sockLock.unlock();
            std::string message = sendBuffer.front();
            bufferLock.unlock();
            if (SendDataFromBuffer(message)) {
                bufferLock.lock();
                sendBuffer.erase(sendBuffer.begin());
                bufferLock.unlock();
            }

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

bool ClientServiceLink::SendDataFromBuffer(const std::string& message) {
    int sock_;
    {
        std::lock_guard<std::mutex> lock(sockMutex);
        sock_ = sock;
    }
    if (sock_ <= 0) {
        std::cerr << "Socket is not connected\n";
        return false;
    }

    ssize_t bytesSent = send(sock_, message.c_str(), message.length(), 0);
    if (bytesSent == -1) {
        std::cerr << "Failed to send message to the service." << std::endl;
        return false;
    }
    return true;
}

void ClientServiceLink::StartClient(const std::string& dir) {
    SettingsManager::LoadSettings(dir + "/config.json");

    serviceId = SettingsManager::GetSettings().serviceId;

    std::thread messageThread(ProcessMessages);
    std::thread sendThread(ProcessSendBuffer);

    std::cout << "Service link client started\n";

    while (true) {
        if (!ConnectToTcpServer(SettingsManager::GetSettings().ip, SettingsManager::GetSettings().port)) {
            std::cerr << "Can not connect to server. Retrying in 5 seconds...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        SendData("CONNECT");
        HandleConnection();

        DisconnectFromTcpServer();
        std::cerr << "Disconnected from serice link server.\n";
    }

    messageThread.join();
}

void ClientServiceLink::ProcessMessages() {
    while (true) {
        std::unique_lock<std::mutex> lock(bufferMutex);
        if (!messageBuffer.empty()) {
            std::string msg = messageBuffer.front();
            messageBuffer.erase(messageBuffer.begin());
            lock.unlock();

            HandleMessageContent(msg);
        } else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void ClientServiceLink::HandleMessageContent(const std::string& message) {
    return;
}
