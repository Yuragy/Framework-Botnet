#ifndef BOTNETMANAGER_H
#define BOTNETMANAGER_H

#include "Bot.h"
#include <vector>
#include <map>
#include <mutex>
#include <future>
#include <string>
#include <nlohmann/json.hpp>

class BotNetManager {
public:
    void addBot(const Bot &bot);
    void removeBot(const std::string &id);
    void updateBot(const std::string &id, Bot::TaskType taskType, const std::map<std::string, std::string> &params);
    void startAllBots();
    void stopAllBots();
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
    void alertOnCriticalEvents(const std::string &contactInfo);
    void scheduleTask(const std::string &id, const std::chrono::system_clock::time_point &time);
    void encryptConfigData(const std::string &filePath);

private:
    std::vector<Bot> bots;
    std::map<std::string, Bot::State> taskStates;
    std::mutex botsMutex;
    std::vector<std::string> clusterNodes;

    void logBotStatus(const std::string &id);
    bool areDependenciesCompleted(const std::string &id);
    void saveReportToJson(const std::string &filePath);
    void saveReportToXml(const std::string &filePath);
    void saveReportToCsv(const std::string &filePath);
};

#endif // BOTNETMANAGER_H



