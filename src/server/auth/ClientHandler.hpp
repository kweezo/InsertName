#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/thread_pool.hpp>

#include <queue>
#include <mutex>
#include <string>
#include <atomic>

#include "common/TypeUtils.hpp"


class ClientHandler {
public:
    static void Init(unsigned short port);
    static void Start();

    static void InitiateShutdown();
    static void Shutdown();

    template<typename... Args>
    static void SendData(const Args&... args);

private:
    static void AcceptConnections();
    static void RecieveData();
    static void ProcessData();
    static void SendDataFromBuffer();

    static void ProcessDataContent(std::string data);

    static boost::asio::io_context io_context;
    static boost::asio::ssl::context ssl_context;
    static boost::asio::ip::tcp::acceptor acceptor;
    static std::vector<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> clientSockets;
    static std::mutex clientSocketsMutex;

    static std::queue<std::string> recieveBuffer;
    static std::mutex recieveBufferMutex;

    static std::queue<std::string> sendBuffer;
    static std::mutex sendBufferMutex;

    static std::atomic<bool> running;
    static std::atomic<bool> shutdown;
    static const int maxThreads = 1024;

    static boost::asio::thread_pool threadPool;
};

// ---------------------------- Template functions ---------------------------- //

template<typename... Args>
void ClientHandler::SendData(const Args&... args) {
    std::string msg = TypeUtils::stickParams(args...);
    std::lock_guard<std::mutex> sendLock(sendBufferMutex);
    sendBuffer.push(msg);
}
