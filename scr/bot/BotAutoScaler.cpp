#include "BotAutoScaler.h"
#include "Logger.h"
#include "Metrics.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <future>

BotAutoScaler::BotAutoScaler(int maxBots) 
    : maxBots(maxBots), cpuLimit(0), memoryLimit(0), parallelTaskLimit(0) {}

void BotAutoScaler::scaleUp(int count) {
    std::lock_guard<std::mutex> lock(botsMutex);
    logAction("Scaling up: " + std::to_string(count) + " bots.");
    try {
        for (int i = 0; i < count; ++i) {
            if (bots.size() < maxBots) {
                Bot newBot("bot_" + std::to_string(bots.size()), Bot::DOWNLOAD, Bot::MEDIUM, {}, {});
                newBot.setResourceLimits(cpuLimit, memoryLimit);
                newBot.setParallelTaskLimit(parallelTaskLimit);
                bots.push_back(newBot);
            } else {
                logAction("Maximum bot limit reached.");
                break;
            }
        }
        updateMetrics();
    } catch (const std::exception &e) {
        handleException(e);
    }
}

std::future<void> BotAutoScaler::scaleUpAsync(int count) {
    return std::async(std::launch::async, &BotAutoScaler::scaleUp, this, count);
}

void BotAutoScaler::scaleDown(int count) {
    std::lock_guard<std::mutex> lock(botsMutex);
    logAction("Scaling down: " + std::to_string(count) + " bots.");
    try {
        for (int i = 0; i < count && !bots.empty(); ++i) {
            bots.pop_back();
        }
        updateMetrics();
    } catch (const std::exception &e) {
        handleException(e);
    }
}

std::future<void> BotAutoScaler::scaleDownAsync(int count) {
    return std::async(std::launch::async, &BotAutoScaler::scaleDown, this, count);
}

std::vector<Bot> BotAutoScaler::getBots() const {
    std::lock_guard<std::mutex> lock(botsMutex);
    return bots;
}

std::string BotAutoScaler::getStatus() const {
    std::lock_guard<std::mutex> lock(botsMutex);
    return "Current bot count: " + std::to_string(bots.size()) + "/" + std::to_string(maxBots) +
           ", CPU limit: " + std::to_string(cpuLimit) + ", Memory limit: " + std::to_string(memoryLimit) +
           ", Parallel task limit: " + std::to_string(parallelTaskLimit);
}

void BotAutoScaler::setResourceLimits(int cpuLimit, int memoryLimit) {
    this->cpuLimit = cpuLimit;
    this->memoryLimit = memoryLimit;
}

void BotAutoScaler::setParallelTaskLimit(int limit) {
    this->parallelTaskLimit = limit;
}

void BotAutoScaler::setMaxBots(int maxBots) {
    this->maxBots = maxBots;
}

void BotAutoScaler::saveState(const std::string &filePath) const {
    std::lock_guard<std::mutex> lock(botsMutex);
    logAction("Saving state to file: " + filePath);
    nlohmann::json state;
    state["maxBots"] = maxBots;
    state["cpuLimit"] = cpuLimit;
    state["memoryLimit"] = memoryLimit;
    state["parallelTaskLimit"] = parallelTaskLimit;
    state["bots"] = bots.size();
    std::ofstream file(filePath);
    file << state.dump(4);
}

void BotAutoScaler::loadState(const std::string &filePath) {
    std::lock_guard<std::mutex> lock(botsMutex);
    logAction("Loading state from file: " + filePath);
    std::ifstream file(filePath);
    nlohmann::json state;
    file >> state;
    maxBots = state["maxBots"];
    cpuLimit = state["cpuLimit"];
    memoryLimit = state["memoryLimit"];
    parallelTaskLimit = state["parallelTaskLimit"];
    bots.clear();
    int botCount = state["bots"];
    for (int i = 0; i < botCount; ++i) {
        bots.emplace_back("bot_" + std::to_string(i), Bot::DOWNLOAD, Bot::MEDIUM, {}, {});
    }
    updateMetrics();
}

void BotAutoScaler::loadConfig(const std::string &configPath) {
    std::lock_guard<std::mutex> lock(botsMutex);
    logAction("Loading configuration from file: " + configPath);
    std::ifstream file(configPath);
    nlohmann::json config;
    file >> config;
    maxBots = config["maxBots"];
    cpuLimit = config["cpuLimit"];
    memoryLimit = config["memoryLimit"];
    parallelTaskLimit = config["parallelTaskLimit"];
    updateMetrics();
}

void BotAutoScaler::logAction(const std::string &message) const {
    Logger::log(Logger::INFO, "BotAutoScaler: " + message);
}

void BotAutoScaler::updateMetrics() const {
    Metrics::updateGauge("bot_count", bots.size());
    Metrics::updateGauge("cpu_limit", cpuLimit);
    Metrics::updateGauge("memory_limit", memoryLimit);
}

void BotAutoScaler::handleException(const std::exception &e) const {
    Logger::log(Logger::ERROR, "Exception in BotAutoScaler: " + std::string(e.what()));
}


