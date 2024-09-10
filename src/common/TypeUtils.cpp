#include "TypeUtils.hpp"

#include <charconv>

namespace TypeUtils {

bool tryPassDouble(const std::string& s, double& d) {
    const char* str = s.c_str();
    char* end;
    d = std::strtod(str, &end);
    return end != str && *end == '\0';
}

bool tryPassInt(const std::string& s, int& i) {
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), i);
    return ec == std::errc() && ptr == s.data() + s.size();
}

std::string getFirstParam(std::string& message) {
    size_t pos = message.find(static_cast<char>(30));
    
    if (pos == std::string::npos) {
        std::string result = message;
        message.clear();
        return result;
    }
    
    std::string firstParam = message.substr(0, pos);
    
    message.erase(0, pos + 1);
    
    return firstParam;
}

}
