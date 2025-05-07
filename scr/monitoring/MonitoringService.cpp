#include "MonitoringService.h"
#include "Logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <curl/curl.h>

MonitoringService::MonitoringService() : monitoringActive(false), collectionInterval(5), alertNotificationMethod("console") {}

MonitoringService::~MonitoringService() {
    stopMonitoring();
}

void MonitoringService::startMonitoring() {
    if (monitoringActive) {
        log("Monitoring already active");
        return;
    }

    log("Starting monitoring");
    monitoringActive = true;
    monitoringTask = std::async(std::launch::async, &MonitoringService::monitoringLoop, this);
}

void MonitoringService::stopMonitoring() {
    if (!monitoringActive) {
        log("Monitoring not active");
        return;
    }

    log("Stopping monitoring");
    monitoringActive = false;
    cv.notify_all();
    if (monitoringTask.valid()) {
        monitoringTask.get();
    }
}

void MonitoringService::collectMetrics() {
    try {
        for (const auto &collector : metricCollectors) {
            collectedMetrics[collector.first] = collector.second();
        }
        log("Collected metrics");
        checkAlerts();
    } catch (const std::exception &e) {
        handleError(e.what());
    }
}

void MonitoringService::setMetricCollectionInterval(int seconds) {
    collectionInterval = seconds;
}

void MonitoringService::registerMetric(const std::string &name, const std::function<std::string()> &collectFunc) {
    metricCollectors[name] = collectFunc;
}

std::map<std::string, std::string> MonitoringService::getCollectedMetrics() const {
    return collectedMetrics;
}

void MonitoringService::registerAlert(const std::string &metric, const std::string &condition, const std::function<void()> &callback) {
    alerts[metric] = {condition, callback};
}

void MonitoringService::setAlertNotificationMethod(const std::string &notificationMethod) {
    alertNotificationMethod = notificationMethod;
}

void MonitoringService::monitoringLoop() {
    while (monitoringActive) {
        collectMetrics();
        std::unique_lock<std::mutex> lock(cv_m);
        cv.wait_for(lock, std::chrono::seconds(collectionInterval), [this]{ return !monitoringActive; });
    }
}

void MonitoringService::log(const std::string &message) const {
    Logger::log(Logger::INFO, message);
}

void MonitoringService::handleError(const std::string &error) const {
    Logger::log(Logger::ERROR, "Monitoring error: " + error);
}

void MonitoringService::checkAlerts() {
    for (const auto &alert : alerts) {
        const auto &metric = alert.first;
        const auto &condition = alert.second.first;
        const auto &callback = alert.second.second;

        if (collectedMetrics.find(metric) != collectedMetrics.end()) {
            std::istringstream iss(collectedMetrics[metric]);
            double value;
            iss >> value;
            if (condition == "HIGH" && value > 80.0) {
                callback();
                sendNotification("Alert: " + metric + " is HIGH (" + std::to_string(value) + ")");
            } else if (condition == "LOW" && value < 20.0) {
                callback();
                sendNotification("Alert: " + metric + " is LOW (" + std::to_string(value) + ")");
            }
        }
    }
}

void MonitoringService::sendNotification(const std::string &message) const {
    if (alertNotificationMethod == "console") {
        std::cout << message << std::endl;
    } else if (alertNotificationMethod == "email") {
        sendEmail(message);
    } else if (alertNotificationMethod == "sms") {
        sendSMS(message);
    } else if (alertNotificationMethod == "messenger") {
        sendToMessenger(message);
    }
}

void MonitoringService::sendEmail(const std::string &message) const {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.example.com");
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "alert@example.com");

        struct curl_slist *recipients = nullptr;
        recipients = curl_slist_append(recipients, "recipient@example.com");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        std::string payload = "To: recipient@example.com\r\nFrom: alert@example.com\r\nSubject: Alert\r\n\r\n" + message;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, nullptr);
        curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            handleError("cURL failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

void MonitoringService::sendSMS(const std::string &message) const {
    // Send sms api logic
    log("SMS sent: " + message);
}

void MonitoringService::sendToMessenger(const std::string &message) const {
     // auto 
    log("Messenger message sent: " + message);
}

void MonitoringService::sendReport() {
    // Send report logic api or
    log("Report sent");
}




