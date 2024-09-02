#pragma once

#include "defines.hpp"
#include "shared/ClientServiceLink.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include <queue>
#include <mutex>
#include <thread>
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <condition_variable>


class ClientHandler {
public:
    static void Init(unsigned short port);
    static void Start();

    template<typename... Args>
    static void SendData(const Args&... args);

private:
    static void AcceptConnections();
    static void ReceiveData();
    static void ProcessData();
    static void SendDataFromBuffer();

    static void ProcessDataContent(std::string data);

    template<typename T>
    static void addToStream(std::stringstream& ss, const T& value);
    template<typename T, typename... Args>
    static void addToStream(std::stringstream& ss, const T& first, const Args&... args);
    template<typename... Args>
    static std::string CreateMessage(const Args&... args);

    static boost::asio::io_context io_context;
    static boost::asio::ssl::context ssl_context;
    static boost::asio::ip::tcp::acceptor acceptor;
    static std::vector<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> clientSockets;
    static std::mutex clientSocketsMutex;

    static std::queue<std::string> receiveBuffer;
    static std::mutex receiveBufferMutex;
    static std::condition_variable receiveBufferCond;

    static std::queue<std::string> sendBuffer;
    static std::mutex sendBufferMutex;
    static std::condition_variable sendBufferCond;

    static std::atomic<bool> running;
    static const int maxThreads = 1024;

    static boost::asio::thread_pool threadPool;
};

// ---------------------------- Template functions ---------------------------- //

template<typename... Args>
void ClientHandler::SendData(const Args&... args) {
    std::string msg = CreateMessage(args...);
    std::lock_guard<std::mutex> sendLock(sendBufferMutex);
    sendBuffer.push(msg);
    sendBufferCond.notify_one();
}

template<typename T>
void ClientHandler::addToStream(std::stringstream& ss, const T& value) {
    ss << value;
}

template<typename T, typename... Args>
void ClientHandler::addToStream(std::stringstream& ss, const T& first, const Args&... args) {
    ss << first << static_cast<char>(30);
    addToStream(ss, args...);
}

template<typename... Args>
std::string ClientHandler::CreateMessage(const Args&... args) {
    std::stringstream ss;
    addToStream(ss, args...);
    return ss.str();
}
