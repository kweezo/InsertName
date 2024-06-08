#pragma once


#include "Config.hpp"
#include "buildConfig.hpp"

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

//#include <ctime>
#include <mutex>
#include <string>
//#include <chrono>
//#include <sstream>
#include <iomanip>
#include <cstring>
#include <unordered_map>

#include <pqxx/pqxx>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <glm/glm.hpp>



class ClientHandler {
public:
    void handleConnection(pqxx::connection& c, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, fd_set& readfds);

private:
    std::string handleMsg(const char* receivedData, int dataSize, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c);

    char registerUser(const std::string& username, const std::string& password, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c);
    char loginUser(const std::string& username, const std::string& password, int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex, pqxx::connection& c);

//    char sendMessage(const std::string& receiverUsername, const std::string& message, int clientSocket, SSL* ssl);
//    std::vector<std::pair<std::string, std::string>> getMessages(std::string senderUsername, int clientSocket, SSL* ssl, int offset = 0);
//    std::vector<std::pair<std::string, std::string>> getNewMessages(std::string senderUsername, int clientSocket, SSL* ssl);

    std::string getNextArg(std::string& msg);
    std::string generateSalt();
    std::string generateHash(const std::string& password, const std::string& salt);
    int getUID(int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, std::mutex& mapMutex);
    void cleanupConnection(int clientSocket, std::unordered_map<int, std::pair<int, SSL*>>& UIDs, SSL* ssl, std::mutex& mapMutex, fd_set& readfds);
};
