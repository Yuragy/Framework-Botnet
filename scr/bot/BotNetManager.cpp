#include "BotNetManager.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <future>
#include <thread>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/rand.h>

void BotNetManager::addBot(const Bot &bot) {
    std::lock_guard<std::mutex> lock(botsMutex);
    bots.push_back(bot);
    taskStates[bot.getId()] = Bot::State::IDLE;
    Logger::log(Logger::INFO, "Added bot: " + bot.getId());
}

void BotNetManager::removeBot(const std::string &id) {
    std::lock_guard<std::mutex> lock(botsMutex);
    auto it = std::remove_if(bots.begin(), bots.end(), [&](const Bot &bot) {
        return bot.getId() == id;
    });
    if (it != bots.end()) {
        bots.erase(it, bots.end());
        taskStates.erase(id);
        Logger::log(Logger::INFO, "Removed bot: " + id);
    } else {
        Logger::log(Logger::WARNING, "Bot not found: " + id);
    }
}

void BotNetManager::updateBot(const std::string &id, Bot::TaskType taskType, const std::map<std::string, std::string> &params) {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        if (bot.getId() == id) {
            bot.updateTask(taskType, params);
            Logger::log(Logger::INFO, "Updated bot: " + id);
            return;
        }
    }
    Logger::log(Logger::WARNING, "Bot not found: " + id);
}

void BotNetManager::startAllBots() {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        if (areDependenciesCompleted(bot.getId())) {
            std::thread(&BotNetManager::startBotAsync, this, bot.getId()).detach();
        } else {
            Logger::log(Logger::WARNING, "Dependencies not completed for bot: " + bot.getId());
        }
    }
}

void BotNetManager::stopAllBots() {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        std::thread(&BotNetManager::stopBotAsync, this, bot.getId()).detach();
    }
}

void BotNetManager::monitorBots() {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (const auto &bot : bots) {
        logBotStatus(bot.getId());
    }
}

std::string BotNetManager::getBotStatus(const std::string &id) {
    std::lock_guard<std::mutex> lock(botsMutex);
    auto it = taskStates.find(id);
    if (it != taskStates.end()) {
        return Bot::stateToString(it->second);
    }
    return "Unknown";
}

std::future<void> BotNetManager::startBotAsync(const std::string &id) {
    return std::async(std::launch::async, [this, id]() {
        std::lock_guard<std::mutex> lock(botsMutex);
        auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
        if (it != bots.end()) {
            try {
                it->performTask();
                taskStates[id] = it->getState();
                Logger::log(Logger::INFO, "Started bot: " + id);
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Error starting bot: " + std::string(e.what()));
            }
        } else {
            Logger::log(Logger::WARNING, "Bot not found: " + id);
        }
    });
}

std::future<void> BotNetManager::stopBotAsync(const std::string &id) {
    return std::async(std::launch::async, [this, id]() {
        std::lock_guard<std::mutex> lock(botsMutex);
        auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
        if (it != bots.end()) {
            try {
                it->stop();
                taskStates[id] = it->getState();
                Logger::log(Logger::INFO, "Stopped bot: " + id);
            } catch (const std::exception &e) {
                Logger::log(Logger::ERROR, "Error stopping bot: " + std::string(e.what()));
            }
        } else {
            Logger::log(Logger::WARNING, "Bot not found: " + id);
        }
    });
}

void BotNetManager::pauseBot(const std::string &id) {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        if (bot.getId() == id) {
            bot.pause();
            Logger::log(Logger::INFO, "Paused bot: " + id);
            return;
        }
    }
    Logger::log(Logger::WARNING, "```cpp
Bot not found: " + id);
}

void BotNetManager::resumeBot(const std::string &id) {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        if (bot.getId() == id) {
            bot.resume();
            Logger::log(Logger::INFO, "Resumed bot: " + id);
            return;
        }
    }
    Logger::log(Logger::WARNING, "Bot not found: " + id);
}

void BotNetManager::setTaskPriority(const std::string &id, int priority) {
    std::lock_guard<std::mutex> lock(botsMutex);
    for (auto &bot : bots) {
        if (bot.getId() == id) {
            bot.setPriority(priority);
            Logger::log(Logger::INFO, "Set priority for bot: " + id);
            return;
        }
    }
    Logger::log(Logger::WARNING, "Bot not found: " + id);
}

void BotNetManager::logBotStatus(const std::string &id) {
    auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
    if (it != bots.end()) {
        Logger::log(Logger::INFO, "Bot " + id + " status: " + Bot::stateToString(it->getState()));
    }
}

bool BotNetManager::areDependenciesCompleted(const std::string &id) {
    auto it = std::find_if(bots.begin(), bots.end(), [&](const Bot &bot) { return bot.getId() == id; });
    if (it != bots.end()) {
        return it->areDependenciesCompleted(taskStates);
    }
    return false;
}

void BotNetManager::generateReport(const std::string &format, const std::string &filePath) {
    if (format == "json") {
        saveReportToJson(filePath);
    } else if (format == "xml") {
        saveReportToXml(filePath);
    } else if (format == "csv") {
        saveReportToCsv(filePath);
    } else {
        Logger::log(Logger::ERROR, "Unsupported report format: " + format);
    }
}

void BotNetManager::saveReportToJson(const std::string &filePath) {
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
}

void BotNetManager::saveReportToXml(const std::string &filePath) {
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
}

void BotNetManager::saveReportToCsv(const std::string &filePath) {
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
}

void BotNetManager::setUpCluster(const std::string &clusterConfig) {
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
}

void BotNetManager::balanceLoad() {
    Logger::log(Logger::INFO, "Balancing load across cluster nodes");
    // Реализация балансировки нагрузки
}

void BotNetManager::alertOnCriticalEvents(const std::string &contactInfo) {
    // Реализация системы оповещений при возникновении критических событий
}

void BotNetManager::scheduleTask(const std::string &id, const std::chrono::system_clock::time_point &time) {
    // Реализация системы планирования задач
}

void BotNetManager::encryptConfigData(const std::string &filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        Logger::log(Logger::ERROR, "Failed to open config file for encryption: " + filePath);
        return;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string configData = buffer.str();

    std::string key = "examplekey123456";
    std::string iv = "exampleiv1234567";

    std::string encryptedData;
    encryptedData.resize(configData.size() + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        Logger::log(Logger::ERROR, "Failed to create EVP_CIPHER_CTX");
        return;
    }

    int len;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        Logger::log(Logger::ERROR, "Failed to initialize AES encryption");
        return;
    }

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&encryptedData[0]), &len, reinterpret_cast<const unsigned char*>(configData.c_str()), configData.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        Logger::log(Logger::ERROR, "Failed to update AES encryption");
        return;
    }

    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&encryptedData[0]) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        Logger::log(Logger::ERROR, "Failed to finalize AES encryption");
        return;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    encryptedData.resize(ciphertext_len);

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        Logger::log(Logger::ERROR, "Failed to open config file for writing: " + filePath);
        return;
    }
    outFile << encryptedData;
    outFile.close();
    Logger::log(Logger::INFO, "Config data encrypted and saved to " + filePath);
}



