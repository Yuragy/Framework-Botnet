#include "BotScheduler.h"
#include "Logger.h"
#include <thread>
#include <chrono>
#include <stdexcept>

BotScheduler::BotScheduler() : stopFlag(false), maxConcurrentTasks(4) {
    for (size_t i = 0; i < maxConcurrentTasks; ++i) {
        workers.emplace_back(&BotScheduler::processTasks, this);
    }
    workers.emplace_back(&BotScheduler::processPeriodicTasks, this);
}

BotScheduler::~BotScheduler() {
    stopAllTasks();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void BotScheduler::scheduleTask(Bot &bot, int priority) {
    std::unique_lock<std::mutex> lock(queueMutex);
    taskQueue.push(ScheduledTask(bot, priority));
    taskStatus[bot.getId()] = "Scheduled";
    condition.notify_one();
    Logger::log(Logger::INFO, "Task scheduled for bot: " + bot.getId() + " with priority: " + std::to_string(priority));
}

void BotScheduler::runScheduledTasks() {
    std::unique_lock<std::mutex> lock(queueMutex);
    condition.notify_all();
    Logger::log(Logger::INFO, "Running all scheduled tasks");
}

void BotScheduler::runTaskWithDelay(Bot &bot, int delaySeconds, int priority) {
    std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
    scheduleTask(bot, priority);
}

void BotScheduler::stopAllTasks() {
    std::unique_lock<std::mutex> lock(queueMutex);
    stopFlag.store(true);
    condition.notify_all();
    Logger::log(Logger::INFO, "All tasks have been stopped");
}

void BotScheduler::resumeAllTasks() {
    std::unique_lock<std::mutex> lock(queueMutex);
    stopFlag.store(false);
    condition.notify_all();
    Logger::log(Logger::INFO, "All tasks have been resumed");
}

void BotScheduler::setMaxConcurrentTasks(size_t maxTasks) {
    maxConcurrentTasks = maxTasks;
    Logger::log(Logger::INFO, "Set max concurrent tasks to: " + std::to_string(maxTasks));
}

void BotScheduler::enablePeriodicTask(Bot &bot, int intervalSeconds, int priority) {
    std::unique_lock<std::mutex> lock(queueMutex);
    periodicTaskQueue.push(PeriodicTask(bot, intervalSeconds, priority));
    taskStatus[bot.getId()] = "Periodic";
    condition.notify_one();
    Logger::log(Logger::INFO, "Periodic task scheduled for bot: " + bot.getId() + " with interval: " + std::to_string(intervalSeconds) + " seconds");
}

void BotScheduler::generateReport() {
    std::unique_lock<std::mutex> lock(queueMutex);
    Logger::log(Logger::INFO, "Generating task execution report...");
    for (const auto &entry : taskStats) {
        Logger::log(Logger::INFO, "Bot: " + entry.first + " executed " + std::to_string(entry.second) + " times");
    }
    for (const auto &entry : taskExecutionTimes) {
        Logger::log(Logger::INFO, "Bot: " + entry.first + " average execution time: " + std::to_string(entry.second.count()) + " seconds");
    }
}

void BotScheduler::monitorTasks() {
    // Реализация мониторинга состояния задач
}

void BotScheduler::setTaskPriority(const std::string &botId, int newPriority) {
    std::unique_lock<std::mutex> lock(queueMutex);
    std::priority_queue<ScheduledTask> newQueue;
    while (!taskQueue.empty()) {
        auto task = taskQueue.top();
        taskQueue.pop();
        if (task.bot.getId() == botId) {
            task.priority = newPriority;
        }
        newQueue.push(task);
    }
    taskQueue = std::move(newQueue);
    Logger::log(Logger::INFO, "Task priority for bot: " + botId + " set to: " + std::to_string(newPriority));
}

std::string BotScheduler::getTaskStatus(const std::string &botId) {
    std::unique_lock<std::mutex> lock(queueMutex);
    return taskStatus.count(botId) ? taskStatus[botId] : "Unknown";
}

void BotScheduler::pauseTask(const std::string &botId) {
    std::unique_lock<std::mutex> lock(queueMutex);
    for (auto &task : taskQueue) {
        if (task.bot.getId() == botId) {
            task.bot.pause();
            taskStatus[botId] = "Paused";
            Logger::log(Logger::INFO, "Task paused for bot: " + botId);
            return;
        }
    }
    Logger::log(Logger::WARNING, "Task not found for bot: " + botId);
}

void BotScheduler::resumeTask(const std::string &botId) {
    std::unique_lock<std::mutex> lock(queueMutex);
    for (auto &task : taskQueue) {
        if (task.bot.getId() == botId) {
            task.bot.resume();
            taskStatus[botId] = "Scheduled";
            Logger::log(Logger::INFO, "Task resumed for bot: " + botId);
            return;
        }
    }
    Logger::log(Logger::WARNING, "Task not found for bot: " + botId);
}

void BotScheduler::processTasks() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this] { return !taskQueue.empty() || stopFlag.load(); });
        if (stopFlag.load() && taskQueue.empty()) {
            return;
        }
        auto task = taskQueue.top();
        taskQueue.pop();
        taskStatus[task.bot.getId()] = "Running";
        auto startTime = std::chrono::steady_clock::now();
        lock.unlock();

        try {
            task.bot.performTask();
            auto endTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> executionTime = endTime - startTime;
            std::unique_lock<std::mutex> statsLock(queueMutex);
            taskStats[task.bot.getId()]++;
            taskExecutionTimes[task.bot.getId()] += executionTime;
            taskStatus[task.bot.getId()] = "Completed";
            Logger::log(Logger::INFO, "Task completed for bot: " + task.bot.getId());
            Logger::log(Logger::INFO, "Task execution time for bot " + task.bot.getId() + ": " + std::to_string(executionTime.count()) + " seconds");
        } catch (const std::exception &e) {
            taskStatus[task.bot.getId()] = "Failed";
            Logger::log(Logger::ERROR, "Error executing task for bot: " + task.bot.getId() + " - " + e.what());
        }
    }
}

void BotScheduler::processPeriodicTasks() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this] { return !periodicTaskQueue.empty() || stopFlag.load(); });
        if (stopFlag.load() && periodicTaskQueue.empty()) {
            return;
        }

        while (!periodicTaskQueue.empty() && std::chrono::steady_clock::now() >= periodicTaskQueue.top().nextRunTime) {
            auto task = periodicTaskQueue.top();
            periodicTaskQueue.pop();
            taskStatus[task.bot.getId()] = "Running";
            auto startTime = std::chrono::steady_clock::now();
            lock.unlock();

            try {
                task.bot.performTask();
                auto endTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> executionTime = endTime - startTime;
                std::unique_lock<std::mutex> statsLock(queueMutex);
                taskStats[task.bot.getId()]++;
                taskExecutionTimes[task.bot.getId()] += executionTime;
                taskStatus[task.bot.getId()] = "Completed";
                Logger::log(Logger::INFO, "Periodic task completed for bot: " + task.bot.getId());
                Logger::log(Logger::INFO, "Periodic task execution time for bot " + task.bot.getId() + ": " + std::to_string(executionTime.count()) + " seconds");
            } catch (const std::exception &e) {
                taskStatus[task.bot.getId()] = "Failed";
                Logger::log(Logger::ERROR, "Error executing periodic task for bot: " + task.bot.getId() + " - " + e.what());
            }

            task.nextRunTime = std::chrono::steady_clock::now() + std::chrono::seconds(task.intervalSeconds);
            lock.lock();
            periodicTaskQueue.push(task);
        }
    }
}







