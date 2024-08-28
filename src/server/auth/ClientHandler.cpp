#include "ClientHandler.hpp"

boost::asio::io_context ClientHandler::io_context;
boost::asio::ssl::context ClientHandler::ssl_context(boost::asio::ssl::context::sslv23);
boost::asio::ip::tcp::acceptor ClientHandler::acceptor(io_context);
std::vector<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> ClientHandler::clientSockets;
std::mutex ClientHandler::clientSocketsMutex;

std::queue<std::string> ClientHandler::receiveBuffer;
std::mutex ClientHandler::receiveBufferMutex;
std::condition_variable ClientHandler::receiveBufferCond;

std::queue<std::string> ClientHandler::processBuffer;
std::mutex ClientHandler::processBufferMutex;
std::condition_variable ClientHandler::processBufferCond;

std::queue<std::string> ClientHandler::sendBuffer;
std::mutex ClientHandler::sendBufferMutex;
std::condition_variable ClientHandler::sendBufferCond;

boost::thread_group ClientHandler::receiveThreadPool;
boost::thread_group ClientHandler::processThreadPool;
boost::thread_group ClientHandler::sendThreadPool;

std::atomic<bool> ClientHandler::running(true);


void ClientHandler::Init(unsigned short port) {
    const std::string certFile = DIR + "auth/network/server.crt";
    const std::string keyFile = DIR + "auth/network/server.key";
    ssl_context.use_certificate_chain_file(certFile);
    ssl_context.use_private_key_file(keyFile, boost::asio::ssl::context::pem);

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
}

void ClientHandler::Start() {
    std::thread(&ClientHandler::AcceptConnections).detach();
    receiveThreadPool.create_thread(boost::bind(&ClientHandler::ReceiveData));
    processThreadPool.create_thread(boost::bind(&ClientHandler::ProcessData));
    sendThreadPool.create_thread(boost::bind(&ClientHandler::SendData));
}

void ClientHandler::AcceptConnections() {
    while (running) {
        auto socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_context, ssl_context);
        acceptor.async_accept(socket->lowest_layer(), [socket](const boost::system::error_code& error) {
            if (!error) {
                socket->async_handshake(boost::asio::ssl::stream_base::server, [socket](const boost::system::error_code& error) {
                    if (!error) {
                        std::lock_guard<std::mutex> lock(clientSocketsMutex);
                        clientSockets.push_back(socket);
                    }
                });
            }
        });
        io_context.run();
    }
}

void ClientHandler::ReceiveData() {
    while (running) {
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        for (auto& socket : clientSockets) {
            if (socket && socket->lowest_layer().is_open()) {
                auto buffer = std::make_shared<std::vector<char>>(1024);
                socket->async_read_some(boost::asio::buffer(*buffer), [buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (!error) {
                        std::lock_guard<std::mutex> lock(receiveBufferMutex);
                        receiveBuffer.push(std::string(buffer->data(), bytes_transferred));
                        receiveBufferCond.notify_one();
                        if (receiveThreadPool.size() < maxThreads) {
                            receiveThreadPool.create_thread(boost::bind(&ClientHandler::ReceiveData));
                        }
                    }
                });
            }
        }
        io_context.run();
    }
}

void ClientHandler::ProcessData() {
    while (running) {
        std::unique_lock<std::mutex> lock(receiveBufferMutex);
        receiveBufferCond.wait(lock, [] { return !receiveBuffer.empty() || !running; });

        while (!receiveBuffer.empty()) {
            std::string data = receiveBuffer.front();
            receiveBuffer.pop();
            lock.unlock();

            ProcessDataContent(data);

            std::lock_guard<std::mutex> processLock(processBufferMutex);
            processBuffer.push(data);
            processBufferCond.notify_one();

            lock.lock();
        }
    }
}

void ClientHandler::SendData() {
    while (running) {
        std::unique_lock<std::mutex> lock(processBufferMutex);
        processBufferCond.wait(lock, [] { return !processBuffer.empty() || !running; });

        while (!processBuffer.empty()) {
            std::string data = processBuffer.front();
            processBuffer.pop();
            lock.unlock();

            std::lock_guard<std::mutex> sendLock(sendBufferMutex);
            sendBuffer.push(data);
            sendBufferCond.notify_one();

            lock.lock();
        }

        std::unique_lock<std::mutex> sendLock(sendBufferMutex);
        sendBufferCond.wait(sendLock, [] { return !sendBuffer.empty() || !running; });

        while (!sendBuffer.empty()) {
            std::string data = sendBuffer.front();
            sendBuffer.pop();
            sendLock.unlock();

            std::lock_guard<std::mutex> lock(clientSocketsMutex);
            for (auto& socket : clientSockets) {
                if (socket && socket->lowest_layer().is_open()) {
                    boost::asio::async_write(*socket, boost::asio::buffer(data), [](const boost::system::error_code& error, std::size_t) {
                        if (error) {
                            std::cerr << "Error sending data: " << error.message() << std::endl;
                            ClientServiceLink::SendData("L", "Error sending data: " + error.message(), 2);
                        }
                    });
                }
            }

            sendLock.lock();
        }
    }
}

void ClientHandler::ProcessDataContent(std::string data) {
    
}
