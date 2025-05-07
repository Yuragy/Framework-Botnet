#ifndef BOTBASE_H
#define BOTBASE_H

#include <string>
#include <map>
#include <future>
#include <vector>

class BotBase {
public:
    enum TaskType {
        DOWNLOAD,
        UPLOAD,
        COPY,
        DELETE,
        MOVE,
        RENAME,
        EXECUTE,
        ENCRYPT,
        DECRYPT,
        AUTO_DISTRIBUTE,
        PHISHING,
        INSTALL_SOFTWARE
    };

    enum TaskPriority {
        LOW,
        MEDIUM,
        HIGH
    };

    enum State {
        WAITING,
        RUNNING,
        COMPLETED,
        FAILED
    };

    using Dependencies = std::vector<std::string>;

    virtual ~BotBase() = default;

    virtual void performTask() = 0;
    virtual void stop() = 0;
    virtual void updateTask(TaskType taskType, const std::map<std::string, std::string> &params) = 0;
    virtual void setResourceLimits(int cpuLimit, int memoryLimit) = 0;
    virtual void setParallelTaskLimit(int limit) = 0;
    virtual std::string getId() const = 0;
    virtual TaskPriority getPriority() const = 0;
    virtual State getState() const = 0;
    virtual bool areDependenciesCompleted(const std::map<std::string, State> &taskStates) const = 0;
    virtual void setDependencies(const Dependencies &dependencies) = 0;
    virtual void logAction(const std::string &message) = 0;
    virtual std::future<void> performTaskAsync() = 0;
    virtual void handleTaskException(const std::string &error) = 0;
    virtual void saveState(const std::string &filePath) const = 0;
    virtual void loadState(const std::string &filePath) = 0;
};

#endif // BOTBASE_H




