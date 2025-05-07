#include "database/DatabaseManager.h"
#include "Logger.h"
#include "DatabaseManagerTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testDatabaseManager() {
    Logger::log(Logger::INFO, "Starting DatabaseManager tests");

    DatabaseManager manager;

    auto start = std::chrono::high_resolution_clock::now();
    try {
        manager.connect("connection_string");
        Logger::log(Logger::INFO, "Database connected");

        std::string result = manager.executeQuery("SELECT * FROM table");
        Logger::log(Logger::INFO, "Query executed: " + result);
        assert(result == "expected_result");

        manager.disconnect();
        Logger::log(Logger::INFO, "Database disconnected");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during database operations: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "DatabaseManager test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "DatabaseManager test passed!");
}

void runDatabaseManagerPerformanceTest(int numQueries, int numThreads) {
    Logger::log(Logger::INFO, "Starting DatabaseManager performance test");

    DatabaseManager manager;
    manager.connect("connection_string");

    auto executeQueries = [&manager](int numQueries) {
        for (int i = 0; i < numQueries; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                std::string result = manager.executeQuery("SELECT * FROM table WHERE id = " + std::to_string(i));
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Query executed in " + std::to_string(duration.count()) + " seconds: " + result);

                assert(result == "expected_result_" + std::to_string(i));
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during executeQuery: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int queriesPerThread = numQueries / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, executeQueries, queriesPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All queries executed in " + std::to_string(duration.count()) + " seconds");

    manager.disconnect();
    Logger::log(Logger::INFO, "DatabaseManager performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numQueries = config.value("numQueries", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numQueries: " + std::to_string(numQueries) + " and numThreads: " + std::to_string(numThreads));

    runDatabaseManagerPerformanceTest(numQueries, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testDatabaseManager();

            int numQueries = 100;
            int numThreads = 10;
            runDatabaseManagerPerformanceTest(numQueries, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

