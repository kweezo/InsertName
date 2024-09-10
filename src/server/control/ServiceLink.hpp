#pragma once

#include <array>
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <string.h>
#include <condition_variable>
#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
#endif

#include "Log.hpp"
#include "AdminConsole.hpp"
#include "common/TypeUtils.hpp"

#define MAX_CONNECTIONS 4


struct Message {
    int serviceId;
    std::string content;
};

class ServiceLink {
public:
	static void StartTcpServer(int port);
	static void ProcessMessages();
	static void ProcessSendBuffer();
	
	template<typename... Args>
    static void SendData(int serviceId, const Args&... args);

	static void NotifyConnection();

private:
	static void HandleConnection(int socket);
	static void HandleMessageContent(Message msg);
	static bool SendDataFromBuffer(int serviceId, const std::string& message);

	static std::mutex connectionMutex;
	static std::condition_variable connectionCond;
	static int activeConnections;

	static std::array<int, MAX_CONNECTIONS> serviceSockets;
	static std::mutex socketMutex;
	static std::vector<Message> messageBuffer;
	static std::mutex bufferMutex;
	static std::array<std::vector<std::string>, MAX_CONNECTIONS> sendBuffer;
	static std::mutex sendBufferMutex;
};

// ---------------------------- Template functions ---------------------------- //

template<typename... Args>
void ServiceLink::SendData(int serviceId, const Args&... args) {
    std::string msg = TypeUtils::stickParams(args..., (char)4);
    std::lock_guard<std::mutex> lock(sendBufferMutex);
    sendBuffer[serviceId].push_back(msg);
}
