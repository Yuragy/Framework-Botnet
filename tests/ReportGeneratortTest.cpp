#include "reporting/ReportGenerator.h"
#include "Logger.h"
#include "ReportGeneratorTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testReportGenerator() {
    Logger::log(Logger::INFO, "Starting ReportGenerator test");

    ReportGenerator generator;

    auto start = std::chrono::high_resolution_clock::now();
    try {
        generator.generateReport("test_report");
        Logger::log(Logger::INFO, "Report generated: test_report");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during report generation: " + std::string(e.what()));
        assert(false);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "ReportGenerator test completed in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "ReportGenerator test passed!");
}

void runReportGeneratorPerformanceTest(int numReports, int numThreads) {
    Logger::log(Logger::INFO, "Starting ReportGenerator performance test");

    auto generateReports = [](int numReports) {
        ReportGenerator generator;
        
        for (int i = 0; i < numReports; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                std::string reportName = "test_report_" + std::to_string(i);
                generator.generateReport(reportName);
                Logger::log(Logger::INFO, "Report generated: " + reportName);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Report generation completed in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during report generation: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int reportsPerThread = numReports / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, generateReports, reportsPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All reports generated in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "ReportGenerator performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numReports = config.value("numReports", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numReports: " + std::to_string(numReports) + " and numThreads: " + std::to_string(numThreads));

    runReportGeneratorPerformanceTest(numReports, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testReportGenerator();

            int numReports = 100;
            int numThreads = 10;
            runReportGeneratorPerformanceTest(numReports, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

