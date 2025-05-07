#ifndef BOTSCHEDULER_H
#define BOTSCHEDULER_H

#include "Bot.h"
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <map>
#include <thread>
#include <atomic>
#include <chrono>

class BotScheduler {
public:
    BotScheduler();
    ~BotScheduler();

    void scheduleTask(Bot &bot, int priority = 0);
    void runScheduledTasks();
    void runTaskWithDelay(Bot &bot, int delaySeconds, int priority = 0);
    void stopAllTasks();
    void resumeAllTasks();
    void setMaxConcurrentTasks(size_t maxTasks);
    void monitorTasks();
    void enablePeriodicTask(Bot &bot, int intervalSeconds, int priority = 0);
    void generateReport();
    void setTaskPriority(const std::string &botId, int newPriority);
    std::string getTaskStatus(const std::string &botId);
    void pauseTask(const std::string &botId);
    void resumeTask(const std::string &botId);

private:
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
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    size_t maxConcurrentTasks;
    std::map<std::string, int> taskStats;
    std::map<std::string, std::string> taskStatus;
    std::map<std::string, std::chrono::duration<double>> taskExecutionTimes;

    void processTasks();
    void processPeriodicTasks();
    void logTaskExecutionTime(const std::string &botId, const std::chrono::steady_clock::time_point &startTime);
};

#endif // BOTSCHEDULER_H





