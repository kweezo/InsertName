#include "Server.hpp"

#include "AdminConsole.hpp"
#include "Log.hpp"


bool Server::isShuttingDown = false;
bool Server::shutdown = false;
std::unique_ptr<pqxx::connection> Server::c;
std::map<int, std::pair<int, SSL*>> Server::UIDs;
std::mutex Server::mapMutex;
std::map<int, std::mutex> Server::clientMutexes;
ClientHandler Server::clientHandler;
std::string Server::dir;
int Server::port;
SSL_CTX* Server::ctx;

#ifdef _WIN32
    SOCKET Server::serverSocket;
    SOCKET Server::clientSocket;
#else
    int Server::serverSocket;
    int Server::clientSocket;
#endif

void Server::init(int port, const std::string& dir) {
    Server::port = port;
    Server::dir = dir;
    Server::ctx = nullptr;

    #ifndef NO_DB
        // Vzpostavitev povezave z bazo podatkov
        std::string conn_str = "dbname=" + Config::dbname +
                              " user=" + Config::dbuser +
                              " password=" + Config::dbpassword +
                              " hostaddr=" + Config::dbhostaddr +
                              " port=" + Config::dbport;
        c = std::make_unique<pqxx::connection>(conn_str);

        // Ustvarjanje tabel v bazi podatkov
        pqxx::work w(*c);

        std::string sql = "CREATE TABLE IF NOT EXISTS Users ("
                            "UID SERIAL PRIMARY KEY,"
                            "Username TEXT NOT NULL,"
                            "PasswordHash TEXT NOT NULL,"
                            "Salt TEXT NOT NULL,"
                            "CreationDate TEXT NOT NULL);";
        w.exec(sql);

    /*    sql = "CREATE TABLE IF NOT EXISTS Messages ("
                "ID SERIAL PRIMARY KEY,"
                "SenderUsername VARCHAR(255),"
                "ReceiverUsername VARCHAR(255),"
                "Message TEXT,"
                "Timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                "IsRead BOOLEAN DEFAULT FALSE);";
        w.exec(sql);
    */
        w.commit();
    #endif
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

    initNetwork();
}

void Server::stop() {
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
    Log::print(1, "Server is listening on port " + std::to_string(ntohs(serverAddress.sin_port)) + "...");

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
        Log::print(3, "Failed to accept client connection");
    }

    ctx = SSL_CTX_new(TLS_server_method());
    if (ctx == nullptr) {
        // handle error
        return -1;
    }

    // Load the server's private key and certificate
    if (SSL_CTX_use_PrivateKey_file(ctx, (dir + "network/server.key").c_str(), SSL_FILETYPE_PEM) <= 0) {
        Log::print(3, "Failed to load server private key");
        return -1;
    }

    if (SSL_CTX_use_certificate_file(ctx, (dir + "network/server.crt").c_str(), SSL_FILETYPE_PEM) <= 0) {
        Log::print(3, "Failed to load server certificate");
        return -1;
    }

    // Create new SSL connection
    SSL* ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        Log::print(3, "Failed to create new SSL connection");
        return -1;
    }

    // Associate the socket with the SSL connection
    SSL_set_fd(ssl, clientSocket);

    // Perform the SSL/TLS handshake
    int ret = SSL_accept(ssl);
    if (ret != 1) {
        int errorCode = SSL_get_error(ssl, ret);
        Log::print(3, "Failed to perform SSL/TLS handshake, error code: " + std::to_string(errorCode));
        return -1;
    }

    std::lock_guard<std::mutex> lock(mapMutex);
    #ifndef NO_DB
        UIDs[clientSocket] = std::make_pair(0, ssl);
    #else
        UIDs[clientSocket] = std::make_pair(clientSocket, ssl);
    #endif
    
    return clientSocket;
}

void Server::handleClients() {
    fd_set readfds;
    int max_sd, sd;

    while (!shutdown) {
        FD_ZERO(&readfds);

        // add server socket to set only if we are still accepting new clients
        if (!Server::isShuttingDown) {
            FD_SET(serverSocket, &readfds);
        }
        max_sd = serverSocket;

        // add child sockets to set
        for (auto& client : UIDs) {
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
            Log::print(3, "Select error occurred on server socket");
            continue;
        }

        // If something happened on the server socket, then its an incoming connection
        if (!Server::isShuttingDown && FD_ISSET(serverSocket, &readfds)) {
            int clientSocket = acceptClient();
            if (clientSocket < 0) {
                Log::print(3, "Failed to accept client connection");
                continue;
            }
            // Set the client socket to non-blocking mode
            #ifdef _WIN32
                unsigned long mode = 1;
                if (ioctlsocket(clientSocket, FIONBIO, &mode) != 0) {
                    Log::print(3, "Failed to set client socket to non-blocking mode");
                    closesocket(clientSocket);
                    continue;
                }
            #else
                int flags = fcntl(clientSocket, F_GETFL, 0);
                if (flags < 0 || fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
                    Log::print(3, "Failed to set client socket to non-blocking mode");
                    close(clientSocket);
                    continue;
                }
            #endif
        }

        // Else its some IO operation on some other socket
        for (auto& client : UIDs) {
            sd = client.first;
        
            // Lock the client mutex before checking the socket
            std::lock_guard<std::mutex> lock(clientMutexes[sd]);
        
            if (FD_ISSET(sd, &readfds)) {
                std::unique_ptr<ClientHandler> handler = std::make_unique<ClientHandler>();
                handler->handleConnection(*c, sd, UIDs, mapMutex, readfds);
            }
        }
    }
}
