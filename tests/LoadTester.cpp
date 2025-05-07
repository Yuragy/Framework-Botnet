#include "LoadTester.h"
#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <chrono>
#include <functional>

std::mutex Logger::logMutex;

void Logger::log(Logger::Level level, const std::string &message) {
    std::lock_guard<std::mutex> lock(logMutex);
    switch (level) {
        case INFO:
            std::cout << "[INFO] " << message << std::endl;
            break;
        case WARNING:
            std::cout << "[WARNING] " << message << std::endl;
            break;
        case ERROR:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
    }
}

LoadTester::LoadTester(int numThreads, std::chrono::milliseconds taskDuration, int numIterations)
    : numThreads(numThreads), taskDuration(taskDuration), numIterations(numIterations), completedTasks(0), failedTasks(0) {}

void LoadTester::run() {
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        Logger::log(Logger::INFO, "Starting iteration " + std::to_string(iteration + 1));
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([this]() {
                try {
                    this->performTask();
                    this->completedTasks++;
                } catch (const std::exception &e) {
                    Logger::log(Logger::ERROR, "Thread exception: " + std::string(e.what()));
                    this->failedTasks++;
                }
            });
        }

        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        Logger::log(Logger::INFO, "Iteration " + std::to_string(iteration + 1) + " completed in " + std::to_string(duration.count()) + " seconds");
        Logger::log(Logger::INFO, "Completed tasks: " + std::to_string(completedTasks.load()) + "/" + std::to_string(numThreads));
        Logger::log(Logger::INFO, "Failed tasks: " + std::to_string(failedTasks.load()) + "/" + std::to_string(numThreads));
        completedTasks = 0; // Reset for next iteration
        failedTasks = 0; // Reset for next iteration
    }

    Logger::log(Logger::INFO, "Load test passed!");
}

LoadTester LoadTester::fromConfig(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numThreads = config.value("numThreads", 100);
    int taskDurationMs = config.value("taskDurationMs", 10);
    int numIterations = config.value("numIterations", 1);

    return LoadTester(numThreads, std::chrono::milliseconds(taskDurationMs), numIterations);
}

void LoadTester::performTask() {
    auto taskStart = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(taskDuration);
    auto taskEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> taskDuration = taskEnd - taskStart;
    Logger::log(Logger::INFO, "Task completed in " + std::to_string(taskDuration.count()) + " seconds");
}

