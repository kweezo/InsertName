#pragma once

#include "common/TypeUtils.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/container/flat_map.hpp>

#include <queue>
#include <mutex>
#include <chrono>
#include <string>
#include <atomic>
#include <unordered_map>


class ClientHandler {
public:
    static void Init(unsigned short port);
    static void Start();

    static void InitiateShutdown();
    static void Shutdown();

    template<typename... Args>
    static void SendData(long uid, const Args&... args);

    static void AddClient(long uid, std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket);
    static void RemoveClient(long uid);
    static std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> GetSocketByUID(long uid);
    static long GetUIDBySocket(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket);

    static void Disconnect(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket);

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

    static std::queue<TypeUtils::Message> sendBuffer;
    static std::mutex sendBufferMutex;

    static std::unordered_map<long, std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> uidToSocketMap;
    static std::unordered_map<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>, long> socketToUIDMap;
    static std::mutex clientMapsMutex;

    static boost::container::flat_map<
     std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>,
      std::chrono::time_point<std::chrono::steady_clock>> unverifiedSockets;
    static std::mutex unverifiedSocketsMutex;

    static std::atomic<bool> running;
    static std::atomic<bool> shutdown;
    static const int maxThreads = 1024;

    static boost::asio::thread_pool threadPool;
};

// ---------------------------- Template functions ---------------------------- //

template<typename... Args>
void ClientHandler::SendData(long uid, const Args&... args) {
    std::string msg = TypeUtils::stickParams(args...);
    std::lock_guard<std::mutex> sendLock(sendBufferMutex);
    sendBuffer.push({uid, msg});
}
