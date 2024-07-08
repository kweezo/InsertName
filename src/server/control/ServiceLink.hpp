#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <chrono>
#include <mutex>
#include <condition_variable>
#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
#endif

#define MAX_CONNECTIONS 4


struct Message {
    int serviceId;
    std::string content;
};

class ServiceLink {
public:
	static void StartTcpServer(int port);
	static void SendData(int socket, const std::string& message);
	static void ProcessMessages();

private:
	static void HandleConnection(int socket);
	static void HandleMessageContent(Message msg);
	static std::string GetFirstParameter(std::string& message);

	static std::mutex connectionMutex;
	static std::condition_variable connectionCond;
	static int activeConnections;

	static std::array<int, MAX_CONNECTIONS> serviceSockets;
	static std::vector<Message> messageBuffer;
	static std::mutex bufferMutex;
};
