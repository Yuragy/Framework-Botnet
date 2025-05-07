#include "cli/CommandLineInterface.h"
#include "Logger.h"
#include "CommandLineInterfaceTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testCommandLineInterface() {
    Logger::log(Logger::INFO, "Starting CommandLineInterface tests");

    CommandLineInterface cli;
    cli.start();

    auto start = std::chrono::high_resolution_clock::now();
    try {
        std::string result = cli.executeCommand("test command");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "Command executed in " + std::to_string(duration.count()) + " seconds");

        assert(result == "expected result");
        Logger::log(Logger::INFO, "Command execution result is as expected");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during executeCommand: " + std::string(e.what()));
        assert(false);
    }

    cli.stop();
    Logger::log(Logger::INFO, "CommandLineInterface test passed!");
}

void runCommandLineInterfacePerformanceTest(int numCommands, int numThreads) {
    Logger::log(Logger::INFO, "Starting CommandLineInterface performance test");

    CommandLineInterface cli;
    cli.start();

    auto executeCommands = [&cli](int numCommands) {
        for (int i = 0; i < numCommands; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                std::string result = cli.executeCommand("test command " + std::to_string(i));
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Command executed in " + std::to_string(duration.count()) + " seconds");

                assert(result == "expected result " + std::to_string(i));
                Logger::log(Logger::INFO, "Command execution result is as expected for command " + std::to_string(i));
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during executeCommand: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int commandsPerThread = numCommands / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, executeCommands, commandsPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All commands executed in " + std::to_string(duration.count()) + " seconds");
    Logger::log(Logger::INFO, "CommandLineInterface performance test passed!");

    cli.stop();
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numCommands = config.value("numCommands", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numCommands: " + std::to_string(numCommands) + " and numThreads: " + std::to_string(numThreads));

    runCommandLineInterfacePerformanceTest(numCommands, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testCommandLineInterface();

            int numCommands = 100;
            int numThreads = 10;
            runCommandLineInterfacePerformanceTest(numCommands, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

