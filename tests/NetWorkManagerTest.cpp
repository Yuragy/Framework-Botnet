#include "network/NetworkManager.h"
#include "Logger.h"
#include "NetworkManagerTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testNetworkManager() {
    Logger::log(Logger::INFO, "Starting NetworkManager test");

    NetworkManager manager;

    auto start = std::chrono::high_resolution_clock::now();
    try {
        bool isConnected = manager.connect("127.0.0.1", 8080);
        assert(isConnected);
        Logger::log(Logger::INFO, "Connected to server: " + std::to_string(isConnected));

        bool isDataSent = manager.sendData("Hello, world!");
        assert(isDataSent);
        Logger::log(Logger::INFO, "Data sent: " + std::to_string(isDataSent));

        std::string response = manager.receiveData();
        Logger::log(Logger::INFO, "Data received: " + response);
        assert(!response.empty());

        manager.disconnect();
        Logger::log(Logger::INFO, "Disconnected from server");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during network operations: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "NetworkManager test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "NetworkManager test passed!");
}

void runNetworkManagerPerformanceTest(int numConnections, int numThreads) {
    Logger::log(Logger::INFO, "Starting NetworkManager performance test");

    auto performNetworkOperations = [](int numConnections) {
        NetworkManager manager;
        
        for (int i = 0; i < numConnections; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                bool isConnected = manager.connect("127.0.0.1", 8080);
                assert(isConnected);
                Logger::log(Logger::INFO, "Connected to server: " + std::to_string(isConnected));

                bool isDataSent = manager.sendData("Hello, world!");
                assert(isDataSent);
                Logger::log(Logger::INFO, "Data sent: " + std::to_string(isDataSent));

                std::string response = manager.receiveData();
                Logger::log(Logger::INFO, "Data received: " + response);
                assert(!response.empty());

                manager.disconnect();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Network operations completed in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during network operations: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int connectionsPerThread = numConnections / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, performNetworkOperations, connectionsPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All network operations completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "NetworkManager performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numConnections = config.value("numConnections", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numConnections: " + std::to_string(numConnections) + " and numThreads: " + std::to_string(numThreads));

    runNetworkManagerPerformanceTest(numConnections, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testNetworkManager();

            int numConnections = 100;
            int numThreads = 10;
            runNetworkManagerPerformanceTest(numConnections, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

