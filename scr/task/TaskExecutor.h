#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H

#include <string>
#include <functional>
#include <map>
#include <future>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

class TaskExecutor {
public:
    TaskExecutor();
    ~TaskExecutor();

    void addTask(const std::string &taskId, const std::function<void()> &taskFunc, int priority = 0);
    void executeTasks();

    std::future<void> executeTasksAsync();

    void setMaxConcurrentTasks(size_t maxTasks);
    void stopAllTasks();

    std::string getTaskStatus(const std::string &taskId) const;

    void registerTaskHandler(const std::string &taskType, const std::function<void(const std::vector<std::string>&)> &handler);

private:
    struct Task {
        std::string taskId;
        std::function<void()> taskFunc;
        int priority;
        Task(const std::string &id, const std::function<void()> &func, int p) : taskId(id), taskFunc(func), priority(p) {}
        bool operator<(const Task &other) const { return priority < other.priority; }
    };

    std::priority_queue<Task> taskQueue;
    std::vector<std::thread> workers;
    std::map<std::string, std::string> taskStatuses;
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> taskHandlers;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    size_t maxConcurrentTasks;

    void processTasks();
    void log(const std::string &message) const;
    void handleError(const std::string &error) const;
};

#endif // TASKEXECUTOR_H
