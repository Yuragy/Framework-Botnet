#include "BotManager.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <future>
#include <thread>
#include <nlohmann/json.hpp>

void BotManager::addBot(const Bot &bot) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        bots.push_back(bot);
        taskStates[bot.getId()] = Bot::State::IDLE;
        logAndMetric("Added bot: " + bot.getId(), "bots_added");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in addBot: " + std::string(e.what()));
    }
}

void BotManager::removeBot(const std::string &id) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        auto it = std::remove_if(bots.begin(), bots.end(), [&](const Bot &bot) {
            return bot.getId() == id;
        });
        if (it != bots.end()) {
            bots.erase(it, bots.end());
            taskStates.erase(id);
            logAndMetric("Removed bot: " + id, "bots_removed");
        } else {
            Logger::log(Logger::WARNING, "Bot not found: " + id);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in removeBot: " + std::string(e.what()));
    }
}

void BotManager::updateBot(const std::string &id, Bot::TaskType taskType, const std::map<std::string, std::string> &params) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                bot.updateTask(taskType, params);
                logAndMetric("Updated bot: " + id, "bots_updated");
                return;
            }
        }
        Logger::log(Logger::WARNING, "Bot not found: " + id);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in updateBot: " + std::string(e.what()));
    }
}

void BotManager::startAllBots() {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (areDependenciesCompleted(bot.getId())) {
                std::thread(&BotManager::startBotAsync, this, bot.getId()).detach();
            } else {
                Logger::log(Logger::WARNING, "Dependencies not completed for bot: " + bot.getId());
            }
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in startAllBots: " + std::string(e.what()));
    }
}

void BotManager::stopAllBots() {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            std::thread(&BotManager::stopBotAsync, this, bot.getId()).detach();
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in stopAllBots: " + std::string(e.what()));
    }
}

void BotManager::pauseAllBots() {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            bot.pause();
        }
        logAndMetric("Paused all bots", "bots_paused");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in pauseAllBots: " + std::string(e.what()));
    }
}

void BotManager::resumeAllBots() {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            bot.resume();
        }
        logAndMetric("Resumed all bots", "bots_resumed");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in resumeAllBots: " + std::string(e.what()));
    }
}

void BotManager::monitorBots() {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (const auto &bot : bots) {
            logBotStatus(bot.getId());
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in monitorBots: " + std::string(e.what()));
    }
}

std::string BotManager::getBotStatus(const std::string &id) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        auto it = taskStates.find(id);
        if (it != taskStates.end()) {
            return Bot::stateToString(it->second);
        }
        return "Unknown";
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in getBotStatus: " + std::string(e.what()));
        return "Error";
    }
}

std::future<void> BotManager::startBotAsync(const std::string &id) {
    return std::async(std::launch::async, [this, id]() {
        try {
            std::lock_guard<std::mutex> lock(botsMutex);
            auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
            if (it != bots.end()) {
                it->performTask();
                taskStates[id] = it->getState();
                                logAndMetric("Started bot: " + id, "bots_started");
            } else {
                Logger::log(Logger::WARNING, "Bot not found: " + id);
            }
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Error starting bot: " + std::string(e.what()));
            Logger::logMetric("bots_start_failed", 1);
        }
    });
}

std::future<void> BotManager::stopBotAsync(const std::string &id) {
    return std::async(std::launch::async, [this, id]() {
        try {
            std::lock_guard<std::mutex> lock(botsMutex);
            auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
            if (it != bots.end()) {
                it->stop();
                taskStates[id] = it->getState();
                logAndMetric("Stopped bot: " + id, "bots_stopped");
            } else {
                Logger::log(Logger::WARNING, "Bot not found: " + id);
            }
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Error stopping bot: " + std::string(e.what()));
        }
    });
}

void BotManager::pauseBot(const std::string &id) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                bot.pause();
                logAndMetric("Paused bot: " + id, "bots_paused");
                return;
            }
        }
        Logger::log(Logger::WARNING, "Bot not found: " + id);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in pauseBot: " + std::string(e.what()));
    }
}

void BotManager::resumeBot(const std::string &id) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                bot.resume();
                logAndMetric("Resumed bot: " + id, "bots_resumed");
                return;
            }
        }
        Logger::log(Logger::WARNING, "Bot not found: " + id);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in resumeBot: " + std::string(e.what()));
    }
}

void BotManager::setTaskPriority(const std::string &id, int priority) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                bot.setPriority(priority);
                logAndMetric("Set priority for bot: " + id, "bots_priority_set");
                return;
            }
        }
        Logger::log(Logger::WARNING, "Bot not found: " + id);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in setTaskPriority: " + std::string(e.what()));
    }
}

void BotManager::logBotStatus(const std::string &id) {
    try {
        auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
        if (it != bots.end()) {
            Logger::log(Logger::INFO, "Bot " + id + " status: " + Bot::stateToString(it->getState()));
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in logBotStatus: " + std::string(e.what()));
    }
}

bool BotManager::areDependenciesCompleted(const std::string &id) {
    try {
        auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
        if (it != bots.end()) {
            return it->areDependenciesCompleted(taskStates);
        }
        return false;
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in areDependenciesCompleted: " + std::string(e.what()));
        return false;
    }
}

void BotManager::generateReport(const std::string &format, const std::string &filePath) {
    try {
        if (format == "json") {
            saveReportToJson(filePath);
        } else if (format == "xml") {
            saveReportToXml(filePath);
        } else if (format == "csv") {
            saveReportToCsv(filePath);
        } else {
            Logger::log(Logger::ERROR, "Unsupported report format: " + format);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in generateReport: " + std::string(e.what()));
    }
}

void BotManager::saveReportToJson(const std::string &filePath) {
    try {
        nlohmann::json report;
        for (const auto &bot : bots) {
            report[bot.getId()] = {
                {"state", getBotStatus(bot.getId())},
                {"priority", bot.getPriority()},
                {"taskType", bot.getTaskType()},
                {"params", bot.getParams()}
            };
        }
        std::ofstream file(filePath);
        file << report.dump(4);
        Logger::log(Logger::INFO, "Report saved to " + filePath);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in saveReportToJson: " + std::string(e.what()));
    }
}

void BotManager::saveReportToXml(const std::string &filePath) {
    try {
        std::ofstream file(filePath);
        file << "<Bots>\n";
        for (const auto &bot : bots) {
            file << "  <Bot id=\"" << bot.getId() << "\">\n";
            file << "    <State>" << getBotStatus(bot.getId()) << "</State>\n";
            file << "    <Priority>" << bot.getPriority() << "</Priority>\n";
            file << "    <TaskType>" << bot.getTaskType() << "</TaskType>\n";
            file << "    <Params>\n";
            for (const auto &param : bot.getParams()) {
                file << "      <Param name=\"" << param.first << "\">" << param.second << "</Param>\n";
            }
            file << "    </Params>\n";
            file << "  </Bot>\n";
        }
        file << "</Bots>\n";
        Logger::log(Logger::INFO, "Report saved to " + filePath);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in saveReportToXml: " + std::string(e.what()));
    }
}

void BotManager::saveReportToCsv(const std::string &filePath) {
    try {
        std::ofstream file(filePath);
        file << "ID,State,Priority,TaskType,Params\n";
        for (const auto &bot : bots) {
            file << bot.getId() << "," << getBotStatus(bot.getId()) << "," << bot.getPriority() << "," << bot.getTaskType();
            for (const auto &param : bot.getParams()) {
                file << "," << param.first << ":" << param.second;
            }
            file << "\n";
        }
        Logger::log(Logger::INFO, "Report saved to " + filePath);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in saveReportToCsv: " + std::string(e.what()));
    }
}

void BotManager::setUpCluster(const std::string &clusterConfig) {
    try {
        nlohmann::json config;
        std::ifstream configFile(clusterConfig);
        if (configFile.is_open()) {
            configFile >> config;
            for (const auto &node : config["nodes"]) {
                clusterNodes.push_back(node.get<std::string>());
            }
            Logger::log(Logger::INFO, "Cluster setup with nodes: " + std::to_string(clusterNodes.size()));
        } else {
            Logger::log(Logger::ERROR, "Failed to open cluster config file: " + clusterConfig);
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in setUpCluster: " + std::string(e.what()));
    }
}

void BotManager::balanceLoad() {
    try {
        Logger::log(Logger::INFO, "Balancing load across cluster nodes");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in balanceLoad: " + std::string(e.what()));
    }
}

void BotManager::logAndMetric(const std::string &message, const std::string &metricName) {
    try {
        Logger::log(Logger::INFO, message);
        esLogger.log("INFO", message);
        prometheusMetrics.incrementCounter(metricName);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in logAndMetric: " + std::string(e.what()));
    }
}

void BotManager::runScheduledTasks() {
    try {
        std::unique_lock<std::mutex> lock(botsMutex);
                condition.notify_all();
        Logger::log(Logger::INFO, "Running all scheduled tasks");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in runScheduledTasks: " + std::string(e.what()));
    }
}

void BotManager::runTaskWithDelay(const std::string &id, int delaySeconds, int priority) {
    try {
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                taskQueue.push(ScheduledTask(bot, priority));
                break;
            }
        }
        condition.notify_one();
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in runTaskWithDelay: " + std::string(e.what()));
    }
}

void BotManager::enablePeriodicTask(const std::string &id, int intervalSeconds, int priority) {
    try {
        std::lock_guard<std::mutex> lock(botsMutex);
        for (auto &bot : bots) {
            if (bot.getId() == id) {
                periodicTaskQueue.push(PeriodicTask(bot, intervalSeconds, priority));
                taskStates[bot.getId()] = Bot::State::RUNNING;
                condition.notify_one();
                Logger::log(Logger::INFO, "Periodic task scheduled for bot: " + bot.getId() + " with interval: " + std::to_string(intervalSeconds) + " seconds");
                break;
            }
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in enablePeriodicTask: " + std::string(e.what()));
    }
}

void BotManager::setMaxConcurrentTasks(size_t maxTasks) {
    try {
        maxConcurrentTasks = maxTasks;
        Logger::log(Logger::INFO, "Set max concurrent tasks to: " + std::to_string(maxTasks));
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in setMaxConcurrentTasks: " + std::string(e.what()));
    }
}

void BotManager::processTasks() {
    try {
        while (true) {
            std::unique_lock<std::mutex> lock(botsMutex);
            condition.wait(lock, [this] { return !taskQueue.empty() || stopFlag.load(); });
            if (stopFlag.load() && taskQueue.empty()) {
                return;
            }
            auto task = taskQueue.top();
            taskQueue.pop();
            taskStates[task.bot.getId()] = Bot::State::RUNNING;
            lock.unlock();

            try {
                task.bot.performTask();
                {
                    std::lock_guard<std::mutex> statsLock(botsMutex);
                    taskStates[task.bot.getId()] = Bot::State::COMPLETED;
                    logAndMetric("Task completed for bot: " + task.bot.getId(), "tasks_completed");
                }
            } catch (const std::exception &e) {
                taskStates[task.bot.getId()] = Bot::State::FAILED;
                Logger::log(Logger::ERROR, "Error executing task for bot: " + task.bot.getId() + " - " + e.what());
                logAndMetric("Task failed for bot: " + task.bot.getId(), "tasks_failed");
            }
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in processTasks: " + std::string(e.what()));
    }
}

void BotManager::processPeriodicTasks() {
    try {
        while (true) {
            std::unique_lock<std::mutex> lock(botsMutex);
            condition.wait(lock, [this] { return !periodicTaskQueue.empty() || stopFlag.load(); });
            if (stopFlag.load() && periodicTaskQueue.empty()) {
                return;
            }

            while (!periodicTaskQueue.empty() && std::chrono::steady_clock::now() >= periodicTaskQueue.top().nextRunTime) {
                auto task = periodicTaskQueue.top();
                periodicTaskQueue.pop();
                taskStates[task.bot.getId()] = Bot::State::RUNNING;
                lock.unlock();

                try {
                    task.bot.performTask();
                    {
                        std::lock_guard<std::mutex> statsLock(botsMutex);
                        taskStates[task.bot.getId()] = Bot::State::COMPLETED;
                        logAndMetric("Periodic task completed for bot: " + task.bot.getId(), "periodic_tasks_completed");
                    }
                } catch (const std::exception &e) {
                    taskStates[task.bot.getId()] = Bot::State::FAILED;
                    Logger::log(Logger::ERROR, "Error executing periodic task for bot: " + task.bot.getId() + " - " + e.what());
                    logAndMetric("Periodic task failed for bot: " + task.bot.getId(), "periodic_tasks_failed");
                }

                task.nextRunTime = std::chrono::steady_clock::now() + std::chrono::seconds(task.intervalSeconds);
                lock.lock();
                periodicTaskQueue.push(task);
            }
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in processPeriodicTasks: " + std::string(e.what()));
    }
}








