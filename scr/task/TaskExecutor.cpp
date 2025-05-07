#include "TaskExecutor.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <sstream>

TaskExecutor::TaskExecutor() : stopFlag(false), maxConcurrentTasks(4) {
    for (size_t i = 0; i < maxConcurrentTasks; ++i) {
        workers.emplace_back(&TaskExecutor::processTasks, this);
    }
}

TaskExecutor::~TaskExecutor() {
    stopAllTasks();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void TaskExecutor::addTask(const std::string &taskId, const std::function<void()> &taskFunc, int priority) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(Task(taskId, taskFunc, priority));
    taskStatuses[taskId] = "Scheduled";
    condition.notify_one();
    Logger::log(Logger::INFO, "Task added: " + taskId + " with priority " + std::to_string(priority));
}

void TaskExecutor::executeTasks() {
    while (!taskQueue.empty()) {
        processTasks();
    }
}

std::future<void> TaskExecutor::executeTasksAsync() {
    return std::async(std::launch::async, &TaskExecutor::executeTasks, this);
}

void TaskExecutor::setMaxConcurrentTasks(size_t maxTasks) {
    maxConcurrentTasks = maxTasks;
    Logger::log(Logger::INFO, "Set max concurrent tasks to: " + std::to_string(maxTasks));
}

void TaskExecutor::stopAllTasks() {
    stopFlag.store(true);
    condition.notify_all();
    Logger::log(Logger::INFO, "All tasks have been stopped");
}

std::string TaskExecutor::getTaskStatus(const std::string &taskId) const {
    auto it = taskStatuses.find(taskId);
    if (it != taskStatuses.end()) {
        return it->second;
    }
    return "Unknown";
}

void TaskExecutor::registerTaskHandler(const std::string &taskType, const std::function<void(const std::vector<std::string>&)> &handler) {
    taskHandlers[taskType] = handler;
}

void TaskExecutor::processTasks() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this] { return !taskQueue.empty() || stopFlag.load(); });

        if (stopFlag.load() && taskQueue.empty()) {
            return;
        }

        auto task = taskQueue.top();
        taskQueue.pop();
        taskStatuses[task.taskId] = "Running";
        lock.unlock();

        try {
            task.taskFunc();
            taskStatuses[task.taskId] = "Completed";
            Logger::log(Logger::INFO, "Task completed: " + task.taskId);
        } catch (const std::exception &e) {
            taskStatuses[task.taskId] = "Failed";
            Logger::log(Logger::ERROR, "Error executing task: " + task.taskId + " - " + e.what());
        }
    }
}

void TaskExecutor::log(const std::string &message) const {
    Logger::log(Logger::INFO, message);
}

void TaskExecutor::handleError(const std::string &error) const {
    Logger::log(Logger::ERROR, "TaskExecutor error: " + error);
}
