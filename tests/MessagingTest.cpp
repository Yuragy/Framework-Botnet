#include "messaging/EmailSender.h"
#include "messaging/SMSSender.h"
#include "messaging/TelegramSender.h"
#include "Logger.h"
#include "MessagingTest.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <nlohmann/json.hpp>

void testEmailSender() {
    Logger::log(Logger::INFO, "Starting EmailSender test");

    EmailSender sender;
    auto start = std::chrono::high_resolution_clock::now();
    try {
        sender.sendMessage("test@example.com", "Hello, email!");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "Email sent in " + std::to_string(duration.count()) + " seconds");
        assert(true);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during sendMessage: " + std::string(e.what()));
        assert(false);
    }

    Logger::log(Logger::INFO, "EmailSender test passed!");
}

void testSMSSender() {
    Logger::log(Logger::INFO, "Starting SMSSender test");

    SMSSender sender;
    auto start = std::chrono::high_resolution_clock::now();
    try {
        sender.sendMessage("1234567890", "Hello, SMS!");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "SMS sent in " + std::to_string(duration.count()) + " seconds");
        assert(true);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during sendMessage: " + std::string(e.what()));
        assert(false);
    }

    Logger::log(Logger::INFO, "SMSSender test passed!");
}

void testTelegramSender() {
    Logger::log(Logger::INFO, "Starting TelegramSender test");

    TelegramSender sender;
    auto start = std::chrono::high_resolution_clock::now();
    try {
        sender.sendMessage("1234567890", "Hello, Telegram!");
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        Logger::log(Logger::INFO, "Telegram message sent in " + std::to_string(duration.count()) + " seconds");
        assert(true);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught during sendMessage: " + std::string(e.what()));
        assert(false);
    }

    Logger::log(Logger::INFO, "TelegramSender test passed!");
}

void runPerformanceTest(int numMessages, int numThreads) {
    Logger::log(Logger::INFO, "Starting messaging performance test");

    auto sendMessages = [](int numMessages) {
        EmailSender emailSender;
        SMSSender smsSender;
        TelegramSender telegramSender;
        
        for (int i = 0; i < numMessages; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            try {
                emailSender.sendMessage("test" + std::to_string(i) + "@example.com", "Hello, email " + std::to_string(i));
                smsSender.sendMessage("1234567890", "Hello, SMS " + std::to_string(i));
                telegramSender.sendMessage("1234567890", "Hello, Telegram " + std::to_string(i));
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                Logger::log(Logger::INFO, "Messages sent in " + std::to_string(duration.count()) + " seconds");
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Exception caught during sendMessage: " + std::string(e.what()));
                assert(false);
            }
        }
    };

    std::vector<std::future<void>> futures;
    int messagesPerThread = numMessages / numThreads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        futures.emplace_back(std::async(std::launch::async, sendMessages, messagesPerThread));
    }

    for (auto &future : futures) {
        future.get();
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "All messages sent in " + std::to_string(duration.count()) + " seconds");

    Logger::log(Logger::INFO, "Messaging performance test passed!");
}

void loadConfigAndRunTests(const std::string &configPath) {
    std::ifstream file(configPath);
    if (!file) {
        Logger::log(Logger::ERROR, "Failed to open config file");
        throw std::runtime_error("Failed to open config file");
    }

    nlohmann::json config;
    file >> config;

    int numMessages = config.value("numMessages", 100);
    int numThreads = config.value("numThreads", 10);

    Logger::log(Logger::INFO, "Loaded configuration with numMessages: " + std::to_string(numMessages) + " and numThreads: " + std::to_string(numThreads));

    runPerformanceTest(numMessages, numThreads);
}

int main(int argc, char* argv[]) {
    Logger::init("logs/async_log.txt", Logger::INFO);

    try {
        if (argc > 1) {
            std::string configPath = argv[1];
            loadConfigAndRunTests(configPath);
        } else {
            testEmailSender();
            testSMSSender();
            testTelegramSender();

            int numMessages = 100;
            int numThreads = 10;
            runPerformanceTest(numMessages, numThreads);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception caught in main: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

