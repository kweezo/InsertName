#include "ClientHandler.hpp"

boost::asio::io_context ClientHandler::io_context;
boost::asio::ssl::context ClientHandler::ssl_context(boost::asio::ssl::context::tlsv13);
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

boost::thread_group ClientHandler::processThreadPool;

std::atomic<bool> ClientHandler::running(true);

void ClientHandler::Init(unsigned short port) {
    const std::string certFile = DIR + "auth/network/server.crt";
    const std::string keyFile = DIR + "auth/network/server.key";

    if (!std::filesystem::exists(certFile) || !std::filesystem::exists(keyFile)) {
        throw std::runtime_error("Certificate or key file not found");
    }

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
    std::thread(&ClientHandler::ReceiveData).detach();
    processThreadPool.create_thread(boost::bind(&ClientHandler::ProcessData));
    std::thread(&ClientHandler::SendData).detach();

    std::cout << "Running io_context..." << std::endl;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        std::thread([]() {
            while (running) {
                io_context.run();
                io_context.restart();
            }
        }).detach();
    }
}

void ClientHandler::AcceptConnections() {
    #ifdef DEBUG
        std::cout << "Waiting to accept connections..." << std::endl;
    #endif
    auto socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_context, ssl_context);
    acceptor.async_accept(socket->lowest_layer(), [socket](const boost::system::error_code& error) {
        if (!error) {
            #ifdef DEBUG
                std::cout << "Accepted connection" << std::endl;
            #endif
            socket->async_handshake(boost::asio::ssl::stream_base::server, [socket](const boost::system::error_code& error) {
                if (!error) {
                    std::lock_guard<std::mutex> lock(clientSocketsMutex);
                    clientSockets.push_back(socket);
                    #ifdef DEBUG
                        std::cout << "Handshake successful" << std::endl;
                    #endif
                } else {
                    std::cerr << "Handshake failed: " << error.message() << std::endl;
                }
                // Continue accepting new connections regardless of handshake result
                #ifdef DEBUG
                    std::cout << "Ready to accept new connections after handshake" << std::endl;
                #endif
                AcceptConnections();
            });
        } else {
            std::cerr << "Accept failed: " << error.message() << std::endl;
            // Continue accepting new connections
            #ifdef DEBUG
                std::cout << "Ready to accept new connections after accept failure" << std::endl;
            #endif
            AcceptConnections();
        }
    });
}

void ClientHandler::ReceiveData() {
    using lowest_layer_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type;
    std::unordered_map<lowest_layer_type*, bool> readingSockets;
    std::unordered_set<lowest_layer_type*> closedSockets;

    while (running) {
        std::lock_guard<std::mutex> lock(clientSocketsMutex);
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
            auto& socket = *it;
            if (!socket || !socket->lowest_layer().is_open()) {
                it = clientSockets.erase(it);
                continue;
            }

            auto& isReading = readingSockets[&socket->lowest_layer()];
            if (isReading) {
                ++it;
                continue;
            }

            isReading = true;
            auto buffer = std::make_shared<std::vector<char>>(1024);
            auto weak_socket = std::weak_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(socket);
            socket->async_read_some(boost::asio::buffer(*buffer), [buffer, &isReading, weak_socket, &closedSockets](const boost::system::error_code& error, std::size_t bytes_transferred) mutable {
                isReading = false;
                if (auto socket = weak_socket.lock()) {
                    if (!error) {
                        std::lock_guard<std::mutex> lock(receiveBufferMutex);
                        receiveBuffer.push(std::string(buffer->data(), bytes_transferred));
                        receiveBufferCond.notify_one();
                    } else {
                        std::lock_guard<std::mutex> lock(clientSocketsMutex);
                        if (closedSockets.find(&socket->lowest_layer()) == closedSockets.end()) {
                            std::cerr << "Receive failed: " << error.message() << std::endl;
                            closedSockets.insert(&socket->lowest_layer());
                        }
                        if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset || error == boost::asio::error::operation_aborted) {
                            auto it = std::find(clientSockets.begin(), clientSockets.end(), socket);
                            if (it != clientSockets.end()) {
                                clientSockets.erase(it);
                            }
                        }
                    }
                }
            });

            ++it;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Add sleep to reduce CPU usage
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
                        }
                    });
                }
            }

            sendLock.lock();
        }
    }
}

void ClientHandler::ProcessDataContent(std::string data) {
    // Process data here
}
