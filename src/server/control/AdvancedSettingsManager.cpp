#include "AdvancedSettingsManager.hpp"

#include "AdminConsole.hpp"

std::string AdvancedSettingsManager::file;
Config AdvancedSettingsManager::config;
std::mutex AdvancedSettingsManager::mutex;
std::vector<std::variant<int*, std::string*>> AdvancedSettingsManager::settings;


void AdvancedSettingsManager::LoadSettings(const std::string& file_) {
    settings = {
        &config.serviceId,
        &config.controlServicePort,

        &config.logLevel,
        &config.maxLogBufferSize,

        &config.dbname,
        &config.dbuser,
        &config.dbpassword,
        &config.dbhostaddr,
        &config.dbport,
        
        &config.commandPrompt,
        &config.commandWindowHeight
    };

    std::lock_guard<std::mutex> lock(mutex);
    file = file_;

    std::ifstream configFile(file, std::ifstream::binary);
    if (configFile) {
        std::cout << "Loading settings from " << file << std::endl;
        Json::Value root;
        configFile >> root;

        if (root.isMember("serviceId")) {
            config.serviceId = root["serviceId"].asInt();
        }
        if (root.isMember("port")) {
            config.controlServicePort = root["controlServicePort"].asInt();
        }

        if (root.isMember("logLevel")) {
            config.logLevel = root["logLevel"].asInt();
        }
        if (root.isMember("maxLogBufferSize")) {
            config.maxLogBufferSize = root["maxLogBufferSize"].asInt();
        }

        if (root.isMember("dbname")) {
            config.dbname = root["dbname"].asString();
        }
        if (root.isMember("dbuser")) {
            config.dbuser = root["dbuser"].asString();
        }
        if (root.isMember("dbpassword")) {
            config.dbpassword = root["dbpassword"].asString();
        }
        if (root.isMember("dbhostaddr")) {
            config.dbhostaddr = root["dbhostaddr"].asString();
        }
        if (root.isMember("dbport")) {
            config.dbport = root["dbport"].asInt();
        }

        if (root.isMember("commandPrompt")) {
            config.commandPrompt = root["commandPrompt"].asString();
        }
        if (root.isMember("commandWindowHeight")) {
            config.commandWindowHeight = root["commandWindowHeight"].asInt();
        }
    }
}

void AdvancedSettingsManager::SaveSettings() {
    std::lock_guard<std::mutex> lock(mutex);

    Json::Value root;
    root["serviceId"] = config.serviceId;
    root["controlServicePort"] = config.controlServicePort;

    root["logLevel"] = config.logLevel;
    root["maxLogBufferSize"] = config.maxLogBufferSize;

    root["dbname"] = config.dbname;
    root["dbuser"] = config.dbuser;
    root["dbpassword"] = config.dbpassword;
    root["dbhostaddr"] = config.dbhostaddr;
    root["dbport"] = config.dbport;

    root["commandPrompt"] = config.commandPrompt;
    root["commandWindowHeight"] = config.commandWindowHeight;

    std::ofstream configFile(file, std::ofstream::binary);
    configFile << root;

    std::cout << "Settings saved to " << file << std::endl;
}

Config AdvancedSettingsManager::GetSettings() {
    std::lock_guard<std::mutex> lock(mutex);
    return config;
}

void AdvancedSettingsManager::SetSettings(
    std::optional<int> serviceId,
    std::optional<int> port,

    std::optional<int> logLevel,
    std::optional<int> maxLogBufferSize,

    std::optional<std::string> dbname,
    std::optional<std::string> dbuser,
    std::optional<std::string> dbpassword,
    std::optional<std::string> dbhostaddr,
    std::optional<int> dbport,

    std::optional<std::string> commandPrompt,
    std::optional<int> commandWindowHeight) {

    std::lock_guard<std::mutex> lock(mutex);

    if (serviceId.has_value()) {
        config.serviceId = serviceId.value();
    }
    if (port.has_value()) {
        config.controlServicePort = port.value();
    }
    if (logLevel.has_value()) {
        config.logLevel = logLevel.value();
    }
    if (maxLogBufferSize.has_value()) {
        config.maxLogBufferSize = maxLogBufferSize.value();
    }
    if (dbname.has_value()) {
        config.dbname = dbname.value();
    }
    if (dbuser.has_value()) {
        config.dbuser = dbuser.value();
    }
    if (dbpassword.has_value()) {
        config.dbpassword = dbpassword.value();
    }
    if (dbhostaddr.has_value()) {
        config.dbhostaddr = dbhostaddr.value();
    }
    if (dbport.has_value()) {
        config.dbport = dbport.value();
    }
    if (commandPrompt.has_value()) {
        config.commandPrompt = commandPrompt.value();
    }
    if (commandWindowHeight.has_value()) {
        config.commandWindowHeight = commandWindowHeight.value();
    }
}

void AdvancedSettingsManager::SetSetting(size_t index, const std::variant<int, std::string>& value) {
    if (index >= settings.size()) {
        throw std::out_of_range("Index out of range");
    }

    std::visit([index](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            if (auto ptr = std::get_if<int*>(&settings[index])) {
                **ptr = arg;
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (auto ptr = std::get_if<std::string*>(&settings[index])) {
                **ptr = arg;
            }
        }
    }, value);
}

std::string AdvancedSettingsManager::GetSetting(size_t index) {
    if (index >= settings.size()) {
        throw std::out_of_range("Index out of range");
    }

    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(*arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return *arg;
        } else if constexpr (std::is_same_v<T, int*>) {
            return std::to_string(*arg);
        } else if constexpr (std::is_same_v<T, std::string*>) {
            return *arg;
        } else {
            throw std::runtime_error(std::string("Unsupported type: ") + typeid(T).name());
        }
    }, settings[index]);
}

bool AdvancedSettingsManager::TryPassDouble(const std::string& s, double& d) {
    std::istringstream iss(s);
    iss >> d;
    return iss.eof() && !iss.fail();
}

bool AdvancedSettingsManager::TryPassInt(const std::string& s, int& i) {
    std::istringstream iss(s);
    iss >> i;
    return iss.eof() && !iss.fail();
}

bool AdvancedSettingsManager::IsValidIPv4(const std::string& ip) {
    std::regex ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if (std::regex_match(ip, ipRegex)) {
        return true;
    }
    return false;
}
