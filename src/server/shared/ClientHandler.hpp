#pragma once

#include "ClientServiceLink.hpp"
#include "defines.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>

#include <queue>
#include <mutex>
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <condition_variable>


class ClientHandler {
public:
    static void Init(unsigned short port);
    static void Start();

private:
    static void AcceptConnections();
    static void ReceiveData();
    static void ProcessData();
    static void SendData();

    static boost::asio::io_context io_context;
    static boost::asio::ssl::context ssl_context;
    static boost::asio::ip::tcp::acceptor acceptor;
    static std::vector<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> clientSockets;
    static std::mutex clientSocketsMutex;

    static std::queue<std::string> receiveBuffer;
    static std::mutex receiveBufferMutex;
    static std::condition_variable receiveBufferCond;

    static std::queue<std::string> processBuffer;
    static std::mutex processBufferMutex;
    static std::condition_variable processBufferCond;

    static std::queue<std::string> sendBuffer;
    static std::mutex sendBufferMutex;
    static std::condition_variable sendBufferCond;

    static boost::thread_group acceptThreadPool;
    static boost::thread_group receiveThreadPool;
    static boost::thread_group processThreadPool;
    static boost::thread_group sendThreadPool;

    static std::atomic<bool> running;
    static const int maxThreads = 1024;
};
