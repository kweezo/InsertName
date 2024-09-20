#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <string>
#include <cctype>
#include <sstream>


namespace TypeUtils {

struct Message {
    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket;
    std::string content;
};

double getCurrentTimestamp();

std::string generateSalt();
std::string hashString(const std::string& str, const std::string& salt);

bool tryPassDouble(const std::string& s, double& d);
bool tryPassInt(const std::string& s, int& i);
bool tryPassUInt(const std::string& s, unsigned int& i);

bool isValidString(const std::string& s);

std::string getFirstParam(std::string& message);

template<typename T>
void addToStream(std::stringstream& ss, const T& value) {
    ss << value;
}

template<typename T, typename... Args>
void addToStream(std::stringstream& ss, const T& first, const Args&... args) {
    ss << first << static_cast<char>(30);
    addToStream(ss, args...);
}

template<typename... Args>
std::string stickParams(const Args&... args) {
    std::stringstream ss;
    addToStream(ss, args...);
    return ss.str();
}

}
