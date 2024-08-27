#include "NetworkClient.hpp"

boost::asio::io_context NetworkClient::io_context;
boost::asio::ssl::context NetworkClient::ssl_context(boost::asio::ssl::context::sslv23);

NetworkClient::NetworkClient(const std::string& server, unsigned short port)
    : server(server), port(port), running(false) {
    socket = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_context, ssl_context);
}

void NetworkClient::Start() {
    running = true;
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context.load_verify_file(DIR + "network/ca.pem");

    Connect();

    receiveThread = boost::thread(boost::bind(&NetworkClient::ReceiveData, this));
    processThread = boost::thread(boost::bind(&NetworkClient::ProcessData, this));
    sendThread = boost::thread(boost::bind(&NetworkClient::SendData, this));
}

void NetworkClient::Stop() {
    running = false;
    io_context.stop();
    receiveThread.join();
    processThread.join();
    sendThread.join();
}

void NetworkClient::Connect() {
    boost::asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(server, std::to_string(port));
    boost::asio::async_connect(socket->lowest_layer(), endpoints, [this](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint&) {
        if (!error) {
            socket->async_handshake(boost::asio::ssl::stream_base::client, [this](const boost::system::error_code& error) {
                if (!error) {
                    std::cout << "Connected to server" << std::endl;
                } else {
                    std::cerr << "Handshake failed: " << error.message() << std::endl;
                }
            });
        } else {
            std::cerr << "Connect failed: " << error.message() << std::endl;
        }
    });
    io_context.run();
}

void NetworkClient::ReceiveData() {
    while (running) {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        socket->async_read_some(boost::asio::buffer(*buffer), [this, buffer](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                std::lock_guard<std::mutex> lock(receiveBufferMutex);
                receiveBuffer.push(std::string(buffer->data(), bytes_transferred));
                receiveBufferCond.notify_one();
            } else {
                std::cerr << "Receive failed: " << error.message() << std::endl;
            }
        });
        io_context.run();
    }
}

void NetworkClient::ProcessData() {
    while (running) {
        std::unique_lock<std::mutex> lock(receiveBufferMutex);
        receiveBufferCond.wait(lock, [this] { return !receiveBuffer.empty() || !running; });

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

void NetworkClient::SendData() {
    while (running) {
        std::unique_lock<std::mutex> lock(processBufferMutex);
        processBufferCond.wait(lock, [this] { return !processBuffer.empty() || !running; });

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
        sendBufferCond.wait(sendLock, [this] { return !sendBuffer.empty() || !running; });

        while (!sendBuffer.empty()) {
            std::string data = sendBuffer.front();
            sendBuffer.pop();
            sendLock.unlock();

            boost::asio::async_write(*socket, boost::asio::buffer(data), [](const boost::system::error_code& error, std::size_t) {
                if (error) {
                    std::cerr << "Error sending data: " << error.message() << std::endl;
                }
            });

            sendLock.lock();
        }
    }
}

void NetworkClient::SendMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(sendBufferMutex);
    sendBuffer.push(message);
    sendBufferCond.notify_one();
}

void NetworkClient::ProcessDataContent(std::string& data) {
    
}
