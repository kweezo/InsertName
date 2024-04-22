#include "Server.hpp"


Server::Server(int port, const std::string& dir) : port(port), dir(dir), ctx(nullptr), ssl(nullptr) {
    Config::GetInstance().LoadConfig(dir + "/config.txt");

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
                    "Username TEXT PRIMARY KEY NOT NULL,"
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
    // Gracefully close down everything
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
    if (SSL_CTX_use_PrivateKey_file(ctx, (dir + "/network/server.key").c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        // handle error
        return -1;
    }

    if (SSL_CTX_use_certificate_file(ctx, (dir + "/network/server.crt").c_str(), SSL_FILETYPE_PEM) <= 0) {
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

    std::cout << "Accepted client connection\n";
    return clientSocket;  // Return the clientSocket
}

void Server::handleClient(int clientSocket) {
    handlers.push_back(std::make_unique<ClientHandler>(clientSocket, ssl, *c));
    handlers.back()->handleConnection();
    // When you're done with the connection, remove the handler object to close the socket.
    handlers.pop_back();
}