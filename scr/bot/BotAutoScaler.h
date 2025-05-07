#ifndef BOTAUTOSCALER_H
#define BOTAUTOSCALER_H

#include "Bot.h"
#include "Metrics.h"
#include <vector>
#include <future>
#include <string>
#include <mutex>

class BotAutoScaler {
public:
    BotAutoScaler(int maxBots);

    void scaleUp(int count);
    std::future<void> scaleUpAsync(int count);
    void scaleDown(int count);
    std::future<void> scaleDownAsync(int count);

    std::vector<Bot> getBots() const;
    std::string getStatus() const;

    void setResourceLimits(int cpuLimit, int memoryLimit);
    void setParallelTaskLimit(int limit);
    void setMaxBots(int maxBots);

    void saveState(const std::string &filePath) const;
    void loadState(const std::string &filePath);

    void loadConfig(const std::string &configPath);

private:
    std::vector<Bot> bots;
    int maxBots;
    int cpuLimit;
    int memoryLimit;
    int parallelTaskLimit;
    mutable std::mutex botsMutex;

    void logAction(const std::string &message) const;
    void updateMetrics() const;
    void handleException(const std::exception &e) const;
};

#endif // BOTAUTOSCALER_H


