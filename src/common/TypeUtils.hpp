#pragma once

#include <string>
#include <sstream>


namespace TypeUtils {

double getCurrentTimestamp();

std::string generateSalt();
std::string hashString(const std::string& str, const std::string& salt);

bool tryPassDouble(const std::string& s, double& d);
bool tryPassInt(const std::string& s, int& i);

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
