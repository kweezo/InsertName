#include "TypeUtils.hpp"

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

}
