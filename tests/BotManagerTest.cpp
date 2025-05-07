#include "bot/BotManager.h"
#include "Logger.h"
#include "BotManagerTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <nlohmann/json.hpp>
#include <fstream>

void testBotManager() {
    Logger::log(Logger::INFO, "Starting BotManager tests");

    BotManager manager;
    Bot bot("bot1", Bot::DOWNLOAD, Bot::HIGH, {{"url", "http://file.txt"}, {"destination", "/tmp/file.txt"}}, {});
    manager.addBot(bot);

    assert(bot.getState() == Bot::WAITING);
    Logger::log(Logger::INFO, "Initial state of bot is WAITING");

    try {
        auto start = std::chrono::high_resolution_clock::now();
        manager.startAllBots();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "All bots started and completed in " + std::to_string(duration.count()) + " seconds");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during startAllBots: " + std::string(e.what()));
        assert(false);
    }

    assert(bot.getState() == Bot::COMPLETED);
    Logger::log(Logger::INFO, "Final state of bot is COMPLETED");
    Logger::log(Logger::INFO, "BotManager test passed!");
}

void runBotManagerPerformanceTest(int numBots, int numThreads) {
    Logger::log(Logger::INFO, "Starting BotManager performance test");

    BotManager manager;
    std::vector<Bot> bots;

    for (int i = 0; i < numBots; ++i) {
        bots.emplace_back("bot" + std::to_string(i), Bot::DOWNLOAD, Bot::MEDIUM, {{"url", "http://example.com/file" + std::to_string(i) + ".txt"}, {"destination", "/tmp/file" + std::to_string(i) + ".txt"}}, {});
        manager.addBot(bots.back());
    }

    auto performBotTasks = [&manager]() {
        try {
            manager.startAllBots();
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Exception caught during startAllBots: " + std::string(e.what()));
            assert(false);
        }
    };

    std::vector<std::future<void>> futures;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, performBotTasks));
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
    Logger::log(Logger::INFO, "BotManager performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int testPort = config.value("testPort", 8080);
    int numThreads = config.value("numThreads", 4);
    int numBots = config.value("numBots", 100);

    Logger::log(Logger::INFO, "Loaded configuration with testPort: " + std::to_string(testPort) + ", numThreads: " + std::to_string(numThreads) + ", and numBots: " + std::to_string(numBots));

    runBotManagerPerformanceTest(numBots, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    if (argc > 1) {
        try {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
            return 1;
        }
    } else {
        try {
            testBotManager();

            int numBots = 100;
            int numThreads = 10;
            runBotManagerPerformanceTest(numBots, numThreads);
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
            return 1;
        }
    }

    return 0;
}

