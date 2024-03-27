#include "NetworkManager.hpp"
#include <thread>
#include <vector>

int main() {
    NetworkManager networkManager;
    networkManager.initNetwork();

    std::vector<std::thread> clientThreads;

    while (true) {
        networkManager.acceptClient();
        clientThreads.push_back(std::thread(&NetworkManager::handleClientConnection, &networkManager));
    }

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}