#include "security/Encryption.h"
#include "security/Authentication.h"
#include "Logger.h"
#include "SecurityTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testEncryption() {
    Logger::log(Logger::INFO, "Starting Encryption test");

    std::string data = "Hello, world!";
    std::string key = "secret";

    auto start = std::chrono::high_resolution_clock::now();
    try {
        std::string encrypted = Encryption::encrypt(data, key);
        Logger::log(Logger::DEBUG, "Data encrypted: " + encrypted);

        std::string decrypted = Encryption::decrypt(encrypted, key);
        Logger::log(Logger::DEBUG, "Data decrypted: " + decrypted);

        assert(data == decrypted);
        Logger::log(Logger::INFO, "Encryption and decryption successful");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during encryption test: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "Encryption test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "Encryption test passed!");
}

void testAuthentication() {
    Logger::log(Logger::INFO, "Starting Authentication test");

    auto start = std::chrono::high_resolution_clock::now();
    try {
        bool authResult = Authentication::authenticate("user", "password");
        Logger::log(Logger::DEBUG, "Authentication result: " + std::to_string(authResult));

        assert(authResult);
        Logger::log(Logger::INFO, "Authentication successful");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during authentication test: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "Authentication test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "Authentication test passed!");
}

void runPerformanceTests(int numOperations, int numThreads) {
    Logger::log(Logger::INFO, "Starting performance tests");

    auto encryptDecryptOperations = [](int numOperations) {
        std::string data = "Hello, world!";
        std::string key = "secret";
        for (int i = 0; i < numOperations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                std::string encrypted = Encryption::encrypt(data, key);
                std::string decrypted = Encryption::decrypt(encrypted, key);
                assert(data == decrypted);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Encryption and decryption completed in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during encryption/decryption: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    auto authenticationOperations = [](int numOperations) {
        for (int i = 0; i < numOperations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                bool authResult = Authentication::authenticate("user", "password");
                assert(authResult);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Authentication completed in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during authentication: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int operationsPerThread = numOperations / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, encryptDecryptOperations, operationsPerThread));
        futures.emplace_back(std::async(std::launch::async, authenticationOperations, operationsPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All operations completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "Performance tests passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numOperations = config.value("numOperations", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numOperations: " + std::to_string(numOperations) + " and numThreads: " + std::to_string(numThreads));

    runPerformanceTests(numOperations, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testEncryption();
            testAuthentication();

            int numOperations = 100;
            int numThreads = 10;
            runPerformanceTests(numOperations, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

