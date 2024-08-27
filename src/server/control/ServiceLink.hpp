#pragma once

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
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <string.h>
#endif

#include "AdminConsole.hpp"
#include "Log.hpp"

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
	static std::string GetFirstParameter(std::string& message);
	static bool SendDataFromBuffer(int serviceId, const std::string& message);

	template<typename T>
    static void addToStream(std::stringstream& ss, const T& value);
    template<typename T, typename... Args>
    static void addToStream(std::stringstream& ss, const T& first, const Args&... args);
    template<typename... Args>
    static std::string CreateMessage(const Args&... args);

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
    std::string msg = CreateMessage(args...);
    std::lock_guard<std::mutex> lock(sendBufferMutex);
    sendBuffer[serviceId].push_back(msg);
}

template<typename T>
void ServiceLink::addToStream(std::stringstream& ss, const T& value) {
    ss << value;
}

template<typename T, typename... Args>
void ServiceLink::addToStream(std::stringstream& ss, const T& first, const Args&... args) {
    ss << first << static_cast<char>(30);
    addToStream(ss, args...);
}

template<typename... Args>
std::string ServiceLink::CreateMessage(const Args&... args) {
    std::stringstream ss;
    addToStream(ss, args...);
    return ss.str();
}
