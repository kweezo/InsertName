#include "NetworkClient.hpp"

#include "clientDefines.hpp"

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

boost::asio::io_context NetworkClient::io_context;
boost::asio::ssl::context NetworkClient::ssl_context(boost::asio::ssl::context::tlsv13);

NetworkClient::NetworkClient(const std::string& server, unsigned short port)
    : server(server), port(port), running(false) {
    socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_context, ssl_context);
}

void NetworkClient::Start() {
    running = true;
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context.load_verify_file(DIR + "network/ca.pem");

    Connect();

    receiveThread = boost::thread(boost::bind(&NetworkClient::RecieveData, this));
    processThread = boost::thread(boost::bind(&NetworkClient::ProcessData, this));
    sendThread = boost::thread(boost::bind(&NetworkClient::SendData, this));

    io_context.run();
}

void NetworkClient::Stop() {
    running = false;
    io_context.stop();
    receiveThread.join();
    processThread.join();
    sendThread.join();
}

void NetworkClient::Connect() {
    std::cout << "Resolving server address..." << std::endl;
    boost::asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(server, std::to_string(port));

    std::cout << "Attempting to connect to server..." << std::endl;
    boost::asio::connect(socket->lowest_layer(), endpoints);

    std::cout << "Connected to server" << std::endl;
    socket->handshake(boost::asio::ssl::stream_base::client);
    std::cout << "Handshake successful" << std::endl;
}

void NetworkClient::RecieveData() {
    while (running) {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        boost::system::error_code error;
        std::size_t bytes_transferred = socket->read_some(boost::asio::buffer(*buffer), error);
        if (!error) {
            #ifdef DEBUG
                std::cout << "Received data: " << std::string(buffer->data(), bytes_transferred) << "\n";
            #endif
            std::lock_guard<std::mutex> lock(recieveBufferMutex);
            recieveBuffer.push(std::string(buffer->data(), bytes_transferred));
        } else {
            std::cerr << "Receive failed: " << error.message() << std::endl;
        }
    }
}

void NetworkClient::ProcessData() {
    while (running) {
        std::unique_lock<std::mutex> lock(recieveBufferMutex);

        if (recieveBuffer.empty()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        while (!recieveBuffer.empty()) {
            std::string data = recieveBuffer.front();
            recieveBuffer.pop();
            lock.unlock();

            ProcessDataContent(data);

            lock.lock();
        }
    }
}

void NetworkClient::SendData() {
    while (running) {
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

            boost::system::error_code error;
            boost::asio::write(*socket, boost::asio::buffer(data), error);
            if (error) {
                std::cerr << "Error sending data: " << error.message() << std::endl;
            }

            sendLock.lock();
        }
    }
}

void NetworkClient::ProcessDataContent(std::string& data) {
    // Implement your data processing logic here
}
