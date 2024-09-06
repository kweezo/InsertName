#pragma once

#include "clientDefines.hpp"
#include "common/TypeUtils.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ssl.hpp>

#include <queue>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

class NetworkClient {
public:
	NetworkClient(const std::string& server, unsigned short port);
	void Start();
	void Stop();

	template <typename... Args>
	void SendMessage(const Args&... args);

private:
	void Connect();
	void RecieveData();
	void ProcessData();
	void SendData();
	void ProcessDataContent(std::string& data);

	static boost::asio::io_context io_context;
	static boost::asio::ssl::context ssl_context;
	std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket;

	std::queue<std::string> recieveBuffer;
	std::mutex recieveBufferMutex;

	std::queue<std::string> sendBuffer;
	std::mutex sendBufferMutex;

	std::atomic<bool> running;
	boost::thread receiveThread;
	boost::thread processThread;
	boost::thread sendThread;

	std::string server;
	unsigned short port;
};

template <typename... Args>
void NetworkClient::SendMessage(const Args&... args) {
	std::string message = TypeUtils::stickParams(args...);
    std::lock_guard<std::mutex> lock(sendBufferMutex);
    sendBuffer.push(message);
	#ifdef DEBUG
    	std::cerr << "Sent message: " << message << std::endl;
	#endif
}
