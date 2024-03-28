#include "HandleClient.hpp"


std::string HandleClient::dir = "";

HandleClient::HandleClient(const std::string& directory) {
    dir = directory;
}

std::string HandleClient::handleMsg(const std::string& receivedMsg) {
    std::string response = "";
    std::string msg = receivedMsg;

    if (receivedMsg[0] == 'r') {
        username = getNextArg(msg);
        std::string password = getNextArg(msg);
        response = registerUser(username, password);
    } else if (receivedMsg[0] == 'l') {
        response = "l";
    } else {
        response = "e";
    }

    return response;
}

char HandleClient::registerUser(const std::string& username, const std::string& password) {
    std::cout << "Registering user: " << username << std::endl;
    userDir = dir + '/' + username;

    if (std::filesystem::exists(userDir)) {
        return 'e';
    } else {
        std::filesystem::create_directory(userDir);
        std::ofstream passwordFile(userDir + "/password.bin", std::ios::binary);
        if (passwordFile.is_open()) {
            passwordFile.write(password.c_str(), password.size());
            passwordFile.close();
        } else {
            std::cerr << "Failed to create password file\n";
        }
        return 'r';
    }
}

std::string HandleClient::getNextArg(std::string& msg) {
    msg.erase(0, 1);

    size_t pos = msg.find((char)30);
    if (pos == std::string::npos) {
        pos = msg.size();
    }

    std::string arg = msg.substr(0, pos); // Get the string up to the next (char)30
    msg.erase(0, pos); // Erase everything up to and including the next (char)30

    return arg;
}