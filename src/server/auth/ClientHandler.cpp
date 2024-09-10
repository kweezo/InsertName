#include "ClientHandler.hpp"

#include "defines.hpp"
#include "shared/ClientServiceLink.hpp"

#include <boost/asio/post.hpp>

#include <thread>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

boost::asio::io_context ClientHandler::io_context;
boost::asio::ssl::context ClientHandler::ssl_context(boost::asio::ssl::context::tlsv13);
boost::asio::ip::tcp::acceptor ClientHandler::acceptor(io_context);
std::vector<std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> ClientHandler::clientSockets;
std::mutex ClientHandler::clientSocketsMutex;

std::queue<std::string> ClientHandler::recieveBuffer;
std::mutex ClientHandler::recieveBufferMutex;

std::queue<std::string> ClientHandler::sendBuffer;
std::mutex ClientHandler::sendBufferMutex;

std::atomic<bool> ClientHandler::running(true);
std::atomic<bool> ClientHandler::shutdown(false);

boost::asio::thread_pool ClientHandler::threadPool(std::thread::hardware_concurrency());

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
    std::thread(&ClientHandler::RecieveData).detach();
    std::thread(&ClientHandler::ProcessData).detach();
    std::thread(&ClientHandler::SendDataFromBuffer).detach();

    std::cout << "Running io_context..." << std::endl;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        std::thread([]() {
            while (!shutdown) {
                io_context.run();
                io_context.restart();
            }
        }).detach();
    }
}

void ClientHandler::InitiateShutdown() {
    running = false;
}

void ClientHandler::Shutdown() {
    std::lock_guard<std::mutex> lock(clientSocketsMutex);
    shutdown = true;

    for (auto& socket : clientSockets) {
        boost::system::error_code ec;
        socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket->lowest_layer().close(ec);
    }
    clientSockets.clear();
    io_context.stop();
    std::cout << "Server has been shut down." << std::endl;
}

void ClientHandler::AcceptConnections() {
    if (!running) return;

    #ifdef DEBUG
        std::cout << "Waiting to accept connections..." << std::endl;
    #endif
    auto socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_context, ssl_context);
    acceptor.async_accept(socket->lowest_layer(), [socket](const boost::system::error_code& error) {
        if (!running) return;

        if (!error) {
            #ifdef DEBUG
                std::cout << "Accepted connection" << std::endl;
            #endif
            socket->async_handshake(boost::asio::ssl::stream_base::server, [socket](const boost::system::error_code& error) {
                if (!running) return;

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

void ClientHandler::RecieveData() {
    using lowest_layer_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type;
    std::unordered_map<lowest_layer_type*, bool> readingSockets;
    std::unordered_set<lowest_layer_type*> closedSockets;

    while (!shutdown) {
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
                auto socket = weak_socket.lock();
                if (!socket) return;

                if (!error) {
                    std::lock_guard<std::mutex> lock(recieveBufferMutex);
                    recieveBuffer.push(std::string(buffer->data(), bytes_transferred));
                    #ifdef DEBUG
                        std::cout << "Received data from client: " << std::string(buffer->data(), bytes_transferred) << "\n";
                    #endif
                    return;
                }

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
            });

            ++it;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Add sleep to reduce CPU usage
    }
}

void ClientHandler::ProcessData() {
    while (!shutdown) {
        while (true) {
            std::string data;
            {
                std::lock_guard<std::mutex> guard(recieveBufferMutex);
                if (recieveBuffer.empty()) {
                    break;
                }
                data = recieveBuffer.front();
                recieveBuffer.pop();
            }

            boost::asio::post(threadPool, [data]() {
                ProcessDataContent(data);
            });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ClientHandler::SendDataFromBuffer() {
    while (!shutdown) {
        while (true) {
            std::unique_lock<std::mutex> sendLock(sendBufferMutex);
            if (sendBuffer.empty()) {
                sendLock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

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
}

void ClientHandler::ProcessDataContent(std::string data) {
    
}
