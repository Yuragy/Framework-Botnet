#ifndef BOTMANAGER_H
#define BOTMANAGER_H

#include "Bot.h"
#include <vector>
#include <map>
#include <mutex>
#include <future>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "utils/Logger.h"
#include "utils/ElasticSearchLogger.h"
#include "utils/RealTimeMonitor.h"
#include "utils/PrometheusMetrics.h"
#include "network/NetworkManager.h"
#include <queue>
#include <atomic>
#include <condition_variable>

class BotManager {
public:
    void addBot(const Bot &bot);
    void removeBot(const std::string &id);
    void updateBot(const std::string &id, Bot::TaskType taskType, const std::map<std::string, std::string> &params);
    void startAllBots();
    void stopAllBots();
    void pauseAllBots();
    void resumeAllBots();
    void monitorBots();
    std::string getBotStatus(const std::string &id);
    std::future<void> startBotAsync(const std::string &id);
    std::future<void> stopBotAsync(const std::string &id);
    void pauseBot(const std::string &id);
    void resumeBot(const std::string &id);
    void setTaskPriority(const std::string &id, int priority);
    void generateReport(const std::string &format, const std::string &filePath);
    void setUpCluster(const std::string &clusterConfig);
    void balanceLoad();
    void runScheduledTasks();
    void runTaskWithDelay(const std::string &id, int delaySeconds, int priority = 0);
    void enablePeriodicTask(const std::string &id, int intervalSeconds, int priority = 0);
    void setMaxConcurrentTasks(size_t maxTasks);

private:
    std::vector<Bot> bots;
    std::map<std::string, Bot::State> taskStates;
    std::mutex botsMutex;
    std::condition_variable condition;
    int maxPriority;
    ElasticSearchLogger esLogger{"localhost", 9200};
    PrometheusMetrics &prometheusMetrics = PrometheusMetrics::getInstance();
    std::vector<std::string> clusterNodes;

    struct ScheduledTask {
        Bot bot;
        int priority;
        std::chrono::steady_clock::time_point timestamp;
        ScheduledTask(Bot b, int p) : bot(b), priority(p), timestamp(std::chrono::steady_clock::now()) {}
        bool operator<(const ScheduledTask& other) const { return priority < other.priority; }
    };

    struct PeriodicTask {
        Bot bot;
        int intervalSeconds;
        int priority;
        std::chrono::steady_clock::time_point nextRunTime;
        PeriodicTask(Bot b, int interval, int p) : bot(b), intervalSeconds(interval), priority(p), nextRunTime(std::chrono::steady_clock::now() + std::chrono::seconds(interval)) {}
        bool operator<(const PeriodicTask& other) const { return nextRunTime > other.nextRunTime; }
    };

    std::priority_queue<ScheduledTask> taskQueue;
    std::priority_queue<PeriodicTask> periodicTaskQueue;
    std::vector<std::thread> workers;
    std::atomic<bool> stopFlag;
    size_t maxConcurrentTasks = 4;

    void logBotStatus(const std::string &id);
    bool areDependenciesCompleted(const std::string &id);
    void saveReportToJson(const std::string &filePath);
    void saveReportToXml(const std::string &filePath);
    void saveReportToCsv(const std::string &filePath);
    void logAndMetric(const std::string &message, const std::string &metricName);
    void processTasks();
    void processPeriodicTasks();
};

#endif // BOTMANAGER_H









