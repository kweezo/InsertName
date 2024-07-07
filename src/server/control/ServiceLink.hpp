#pragma once

#include "shared/structs.hpp"

#include <iostream>
#include <thread>
#include <vector>
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

class ServiceLink {
public:
	static void StartTcpServer(int port);
	static void HandleConnection(int socket);
	static void SendMessage(int socket, const std::string& message);
	static std::string GetFirstParameter(std::string& message);

private:
	static std::mutex connectionMutex;
	static std::condition_variable connectionCond;
	static int activeConnections;
	static const int maxConnections;

	static std::vector<Message> messageBuffer;
	static std::mutex bufferMutex;
};
