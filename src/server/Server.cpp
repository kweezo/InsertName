#include "Server.hpp"


Server::Server(int port, const std::string& dir) : port(port), dir(dir), ctx(nullptr), ssl(nullptr) {
    // Vzpostavitev povezave z bazo podatkov
    std::string conn_str = "dbname=" + Config::GetInstance().dbname +
                          " user=" + Config::GetInstance().dbuser +
                          " password=" + Config::GetInstance().dbpassword +
                          " hostaddr=" + Config::GetInstance().dbhostaddr +
                          " port=" + Config::GetInstance().dbport;
    c = std::make_unique<pqxx::connection>(conn_str);

    // Ustvarjanje tabel v bazi podatkov
    pqxx::work W(*c);

    std::string sql = "CREATE TABLE IF NOT EXISTS Users ("
                          "UserId SERIAL PRIMARY KEY,"
                          "Username TEXT NOT NULL,"
                          "PasswordHash TEXT NOT NULL,"
                          "Salt TEXT NOT NULL,"
                          "CreationDate TEXT NOT NULL);";
    W.exec(sql);

    sql = "CREATE TABLE IF NOT EXISTS Messages ("
              "ID SERIAL PRIMARY KEY,"
              "SenderUsername VARCHAR(255),"
              "ReceiverUsername VARCHAR(255),"
              "Message TEXT,"
              "Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
              "IsRead BOOLEAN DEFAULT FALSE);";
    W.exec(sql);

    W.commit();

    // Initialize OpenSSL
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();

    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize winsock");
        }
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #else
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #endif

    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }
}

Server::~Server() {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    #ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(serverSocket);
    #endif
}

int Server::initNetwork() {
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // Port number
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 3) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Server is listening on port " << ntohs(serverAddress.sin_port) << "...\n";
    return 0;
}

int Server::acceptClient() {
    sockaddr_in clientAddress{};
    #ifdef _WIN32
        int clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    #else
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    #endif

    if (clientSocket == -1) {
        std::cerr << "Failed to accept client connection";
    }

    ctx = SSL_CTX_new(TLS_server_method());
    if (ctx == nullptr) {
        // handle error
        return -1;
    }

    // Load the server's private key and certificate
    if (SSL_CTX_use_PrivateKey_file(ctx, (dir + "network/server.key").c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        // handle error
        return -1;
    }

    if (SSL_CTX_use_certificate_file(ctx, (dir + "network/server.crt").c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        // handle error
        return -1;
    }

    // Create new SSL connection
    ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        // handle error
        return -1;
    }

    // Associate the socket with the SSL connection
    SSL_set_fd(ssl, clientSocket);

    // Perform the SSL/TLS handshake
    int ret = SSL_accept(ssl);
    if (ret != 1) {
        int errorCode = SSL_get_error(ssl, ret);
        fprintf(stderr, "SSL error: %s\n", ERR_error_string(errorCode, NULL));
        return -1;
    }

    std::lock_guard<std::mutex> lock(mapMutex);
    clientIds[clientSocket] = 0;

    return clientSocket;
}

void Server::handleClients() {
    fd_set readfds;
    int max_sd, sd;

    // Create a thread pool with 128 threads
    ThreadPool pool(128);

    while (true) {
        FD_ZERO(&readfds);

        // add server socket to set
        FD_SET(serverSocket, &readfds);
        max_sd = serverSocket;

        // add child sockets to set
        for (auto& client : clientIds) {
            // socket descriptor
            sd = client.first;

            // if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error");
        }

        // If something happened on the server socket, then its an incoming connection
        if (FD_ISSET(serverSocket, &readfds)) {
            int clientSocket = acceptClient();
        }

        // else its some IO operation on some other socket
        for (auto& client : clientIds) {
            sd = client.first;
        
            if (FD_ISSET(sd, &readfds)) {
                // Add a new job to the thread pool
                pool.enqueue([this, sd, ssl = this->ssl, &c = this->c]() {
                    std::unique_ptr<ClientHandler> handler = std::make_unique<ClientHandler>();
                    handler->handleConnection(ssl, *c, sd, clientIds, mapMutex);
                });
            }
        }
    }
}
