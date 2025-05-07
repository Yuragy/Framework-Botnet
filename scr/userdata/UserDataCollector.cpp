#include "UserDataCollector.h"
#include "Logger.h"
#include "EncryptionUtils.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <thread>

UserDataCollector::UserDataCollector(size_t maxThreads)
    : retryCount(3), timeoutMilliseconds(5000), cacheLifetime(3600), threadPool(maxThreads) {}

void UserDataCollector::collectData(const std::string &userId) {
    Logger::log(Logger::INFO, "Starting data collection for user: " + userId);
    auto start = std::chrono::high_resolution_clock::now();

    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (isCacheValid(userId)) {
            decryptCacheData(cache[userId]);
            logCollectedData(cache[userId], userId);
            Logger::log(Logger::INFO, "Data retrieved from cache for user: " + userId);
            return;
        }
    }

    std::unordered_map<std::string, std::string> collectedData;
    std::vector<std::future<std::unordered_map<std::string, std::string>>> futures;

    for (const auto &dataSource : dataSources) {
        futures.push_back(threadPool.enqueue([this, dataSource, userId]() {
            return attemptCollect(dataSource, userId);
        }));
    }

    for (auto &future : futures) {
        try {
            auto data = future.get();
            std::lock_guard<std::mutex> lock(dataMutex);
            mergeData(collectedData, data);
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Error collecting data from source: " + std::string(e.what()));
        }
    }

    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        encryptCacheData(collectedData);
        cacheData(userId, collectedData);
    }

    logCollectedData(collectedData, userId);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    Logger::log(Logger::INFO, "Completed data collection for user: " + userId + " in " + std::to_string(duration.count()) + " seconds");
}

std::future<void> UserDataCollector::collectDataAsync(const std::string &userId) {
    return std::async(std::launch::async, &UserDataCollector::collectData, this, userId);
}

void UserDataCollector::addDataSource(std::shared_ptr<DataSource> dataSource) {
    dataSources.push_back(dataSource);
    Logger::log(Logger::INFO, "Data source added");
}

void UserDataCollector::removeDataSource(const std::shared_ptr<DataSource>& dataSource) {
    dataSources.erase(std::remove(dataSources.begin(), dataSources.end(), dataSource), dataSources.end());
    Logger::log(Logger::INFO, "Data source removed");
}

void UserDataCollector::setRetryCount(int retries) {
    retryCount = retries;
    Logger::log(Logger::INFO, "Retry count set to: " + std::to_string(retryCount));
}

void UserDataCollector::setTimeout(int milliseconds) {
    timeoutMilliseconds = milliseconds;
    Logger::log(Logger::INFO, "Timeout set to: " + std::to_string(timeoutMilliseconds) + " ms");
}

void UserDataCollector::setCacheLifetime(int seconds) {
    cacheLifetime = seconds;
    Logger::log(Logger::INFO, "Cache lifetime set to: " + std::to_string(cacheLifetime) + " seconds");
}

void UserDataCollector::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
    cacheTimestamps.clear();
    Logger::log(Logger::INFO, "Cache cleared");
}

void UserDataCollector::logCollectedData(const std::unordered_map<std::string, std::string> &collectedData, const std::string &userId) {
    for (const auto &entry : collectedData) {
        Logger::log(Logger::INFO, "Collected data for user " + userId + " - Key: " + entry.first + ", Value: " + entry.second);
    }
}

void UserDataCollector::mergeData(std::unordered_map<std::string, std::string> &collectedData, const std::unordered_map<std::string, std::string> &newData) {
    for (const auto &entry : newData) {
        if (collectedData.find(entry.first) == collectedData.end()) {
            collectedData[entry.first] = entry.second;
        }
    }
}

std::unordered_map<std::string, std::string> UserDataCollector::attemptCollect(const std::shared_ptr<DataSource>& dataSource, const std::string &userId) {
    for (int attempt = 0; attempt < retryCount; ++attempt) {
        try {
            auto future = std::async(std::launch::async, [&dataSource, &userId]() {
                return dataSource->collect(userId);
            });

            if (future.wait_for(std::chrono::milliseconds(timeoutMilliseconds)) == std::future_status::timeout) {
                throw std::runtime_error("Timeout collecting data from source");
            }

            return future.get();
        } catch (const std::exception &e) {
            Logger::log(Logger::WARNING, "Attempt " + std::to_string(attempt + 1) + " failed for user " + userId + " with error: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds((1 << attempt))); // Exponential backoff
        }
    }
    throw std::runtime_error("All attempts to collect data failed for user: " + userId);
}

void UserDataCollector::cacheData(const std::string &userId, const std::unordered_map<std::string, std::string> &data) {
    cache[userId] = data;
    cacheTimestamps[userId] = std::chrono::system_clock::now();
    Logger::log(Logger::INFO, "Data cached for user: " + userId);
}

bool UserDataCollector::isCacheValid(const std::string &userId) {
    auto it = cacheTimestamps.find(userId);
    if (it == cacheTimestamps.end()) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto cacheAge = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
    return cacheAge <= cacheLifetime;
}

void UserDataCollector::encryptCacheData(std::unordered_map<std::string, std::string> &data) {
    std::string key = "your_encryption_key";
    for (auto &entry : data) {
        entry.second = EncryptionUtils::encryptString(entry.second, key);
    }
}

void UserDataCollector::decryptCacheData(std::unordered_map<std::string, std::string> &data) {
    std::string key = "your_encryption_key";
    for (auto &entry : data) {
        entry.second = EncryptionUtils::decryptString(entry.second, key);
    }
}





