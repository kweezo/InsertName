#include "NetworkManager.hpp"


NetworkManager::NetworkManager(const std::string& serverIP, int port)
    : serverIP(serverIP), port(port), sock(INVALID_SOCKET), ctx(nullptr), ssl(nullptr) {
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();

    // Initialize WinSock
    #ifdef _WIN32
        WSAData data;
        WORD ver = MAKEWORD(2, 2);
        int wsResult = WSAStartup(ver, &data);
        if (wsResult != 0) {
            std::cerr << "Can't start Winsock, Err #" << wsResult << '\n';
        }
    #endif
}

NetworkManager::~NetworkManager() {
    // Gracefully close down everything
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    closesocket(sock);
    #ifdef _WIN32
        WSACleanup();
    #endif
}

bool NetworkManager::connectToServer() {
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        #ifdef _WIN32
            std::cerr << "Can't create socket, Err #" << WSAGetLastError() << '\n';
        #else
            std::cerr << "Can't create socket, Err #" << errno << '\n';
        #endif
        return false;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &(hint.sin_addr));

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        #ifdef _WIN32
            std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << '\n';
        #else
            std::cerr << "Can't connect to server, Err #" << errno << '\n';
        #endif
        return false;
    } else {
        std::cout << "Connected to server\n";
    }

    // Create new SSL context
    ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == nullptr) {
        // handle error
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Create new SSL connection
    ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        // handle error
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Associate the socket with the SSL connection
    SSL_set_fd(ssl, sock);

    // Perform the SSL/TLS handshake
    int ret = SSL_connect(ssl);
    while (ret != 1) {
        int errorCode = SSL_get_error(ssl, ret);
        if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE) {
            // The operation did not complete; the same TLS/SSL I/O function should be called again later
            continue;
        } else {
            // handle error
            fprintf(stderr, "SSL error: %s\n", ERR_reason_error_string(errorCode));
            return false;
        }
    }

    return true;
}

std::string NetworkManager::sendData(const std::string& message) {
    // Send the message
    int sendResult = SSL_write(ssl, message.c_str(), message.size() + 1);
    if (sendResult <= 0) {
        // handle error
        return "";
    }

    // Receive the response
    char buf[1024];
    memset(buf, 0, 1024);
    int bytesReceived = SSL_read(ssl, buf, 1024);
    if (bytesReceived > 0) {
        std::string response(buf, 0, bytesReceived);
        std::cout << "SERVER> " << response << '\n';
        return response;
    }
    
    return "";
}