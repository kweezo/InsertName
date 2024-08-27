#pragma once

#include "clientDefines.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <memory>

class NetworkClient {
public:
	NetworkClient(const std::string& server, unsigned short port);
	void Start();
	void Stop();

    void SendMessage(const std::string& message);

private:
	void Connect();
	void ReceiveData();
	void ProcessData();
	void SendData();
	void ProcessDataContent(std::string& data);

	static boost::asio::io_context io_context;
	static boost::asio::ssl::context ssl_context;
	std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket;

	std::queue<std::string> receiveBuffer;
	std::mutex receiveBufferMutex;
	std::condition_variable receiveBufferCond;

	std::queue<std::string> processBuffer;
	std::mutex processBufferMutex;
	std::condition_variable processBufferCond;

	std::queue<std::string> sendBuffer;
	std::mutex sendBufferMutex;
	std::condition_variable sendBufferCond;

	std::atomic<bool> running;
	boost::thread receiveThread;
	boost::thread processThread;
	boost::thread sendThread;

	std::string server;
	unsigned short port;
};
