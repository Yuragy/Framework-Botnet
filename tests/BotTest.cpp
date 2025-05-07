#include "bot/Bot.h"
#include "Logger.h"
#include "BotTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testBot() {
    Logger::log(Logger::INFO, "Starting Bot tests");

    Bot bot("bot1", Bot::DOWNLOAD, Bot::HIGH, {{"url", "http:///file.txt"}, {"destination", "/tmp/file.txt"}}, {});

    assert(bot.getState() == Bot::WAITING);
    Logger::log(Logger::INFO, "Initial state of bot is WAITING");

    try {
        auto start = std::chrono::high_resolution_clock::now();
        bot.performTask();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "Bot completed task in " + std::to_string(duration.count()) + " seconds");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during performTask: " + std::string(e.what()));
        assert(false);
    }

    assert(bot.getState() == Bot::COMPLETED);
    Logger::log(Logger::INFO, "Final state of bot is COMPLETED");
    Logger::log(Logger::INFO, "Bot test passed!");
}

void runBotPerformanceTest(int numBots, int numThreads) {
    Logger::log(Logger::INFO, "Starting Bot performance test");

    std::vector<Bot> bots;
    for (int i = 0; i < numBots; ++i) {
        bots.emplace_back("bot" + std::to_string(i), Bot::DOWNLOAD, Bot::MEDIUM, {{"url", "http:///file" + std::to_string(i) + ".txt"}, {"destination", "/tmp/file" + std::to_string(i) + ".txt"}}, {});
    }

    auto performBotTasks = [&bots](int startIdx, int endIdx) {
        for (int i = startIdx; i < endIdx; ++i) {
            try {
                auto start = std::chrono::high_resolution_clock::now();
                bots[i].performTask();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Bot " + bots[i].getId() + " completed task in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during performTask for bot " + bots[i].getId() + ": " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int botsPerThread = numBots / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        int startIdx = i * botsPerThread;
        int endIdx = (i == numThreads - 1) ? numBots : startIdx + botsPerThread;
        futures.emplace_back(std::async(std::launch::async, performBotTasks, startIdx, endIdx));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    for (const auto &bot : bots) {
        assert(bot.getState() == Bot::COMPLETED);
    }

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All bots completed in " + std::to_string(duration.count()) + " seconds");
    Logger::log(Logger::INFO, "Bot performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numBots = config.value("numBots", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numBots: " + std::to_string(numBots) + ", numThreads: " + std::to_string(numThreads));

    runBotPerformanceTest(numBots, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testBot();

            int numBots = 100;
            int numThreads = 10;
            runBotPerformanceTest(numBots, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}


