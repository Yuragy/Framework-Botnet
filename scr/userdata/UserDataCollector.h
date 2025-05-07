#ifndef USERDATACOLLECTOR_H
#define USERDATACOLLECTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <future>
#include <chrono>
#include "ThreadPool.h"
#include "DataSource.h"

class UserDataCollector {
public:
    UserDataCollector(size_t maxThreads = std::thread::hardware_concurrency());

    void collectData(const std::string &userId);
    std::future<void> collectDataAsync(const std::string &userId);
    void addDataSource(std::shared_ptr<DataSource> dataSource);
    void removeDataSource(const std::shared_ptr<DataSource>& dataSource);
    void setRetryCount(int retries);
    void setTimeout(int milliseconds);
    void setCacheLifetime(int seconds);
    void clearCache();

private:
    std::vector<std::shared_ptr<DataSource>> dataSources;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> cache;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> cacheTimestamps;
    std::mutex cacheMutex;
    std::mutex dataMutex;
    int retryCount;
    int timeoutMilliseconds;
    int cacheLifetime;
    ThreadPool threadPool;

    void logCollectedData(const std::unordered_map<std::string, std::string> &collectedData, const std::string &userId);
    void mergeData(std::unordered_map<std::string, std::string> &collectedData, const std::unordered_map<std::string, std::string> &newData);
    std::unordered_map<std::string, std::string> attemptCollect(const std::shared_ptr<DataSource>& dataSource, const std::string &userId);
    void cacheData(const std::string &userId, const std::unordered_map<std::string, std::string> &data);
    bool isCacheValid(const std::string &userId);
    void encryptCacheData(std::unordered_map<std::string, std::string> &data);
    void decryptCacheData(std::unordered_map<std::string, std::string> &data);
};

#endif // USERDATACOLLECTOR_H






