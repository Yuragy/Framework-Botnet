#include "monitoring/MonitoringService.h"
#include "Logger.h"
#include "MonitoringServiceTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testMonitoringService() {
    Logger::log(Logger::INFO, "Starting MonitoringService test");

    MonitoringService service;

    auto start = std::chrono::high_resolution_clock::now();
    try {
        service.startMonitoring();
        Logger::log(Logger::INFO, "Monitoring started");

        service.collectMetrics();
        Logger::log(Logger::INFO, "Metrics collected");

        service.stopMonitoring();
        Logger::log(Logger::INFO, "Monitoring stopped");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during monitoring operations: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "MonitoringService test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "MonitoringService test passed!");
}

void runMonitoringServicePerformanceTest(int numMetrics, int numThreads) {
    Logger::log(Logger::INFO, "Starting MonitoringService performance test");

    auto collectMetrics = [](int numMetrics) {
        MonitoringService service;
        service.startMonitoring();
        
        for (int i = 0; i < numMetrics; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                service.collectMetrics();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Metrics collected in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during collectMetrics: " + std::string(e.what()));
                assert(false);
            }
        }

        service.stopMonitoring();
    };

    std::vector<std::future<void>> futures;
    int metricsPerThread = numMetrics / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, collectMetrics, metricsPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All metrics collected in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "MonitoringService performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numMetrics = config.value("numMetrics", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numMetrics: " + std::to_string(numMetrics) + " and numThreads: " + std::to_string(numThreads));

    runMonitoringServicePerformanceTest(numMetrics, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testMonitoringService();

            int numMetrics = 100;
            int numThreads = 10;
            runMonitoringServicePerformanceTest(numMetrics, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

