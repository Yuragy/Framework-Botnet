#ifndef MONITORINGSERVICE_H
#define MONITORINGSERVICE_H

#include <string>
#include <future>
#include <map>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>

class MonitoringService {
public:
    MonitoringService();
    ~MonitoringService();
    
    void startMonitoring();
    void stopMonitoring();
    void collectMetrics();
    void setMetricCollectionInterval(int seconds);
    void registerMetric(const std::string &name, const std::function<std::string()> &collectFunc);
    std::map<std::string, std::string> getCollectedMetrics() const;
    void registerAlert(const std::string &metric, const std::string &condition, const std::function<void()> &callback);
    void setAlertNotificationMethod(const std::string &notificationMethod);
    void sendReport();

private:
    std::atomic<bool> monitoringActive;
    int collectionInterval;
    std::map<std::string, std::function<std::string()>> metricCollectors;
    std::map<std::string, std::string> collectedMetrics;
    std::map<std::string, std::pair<std::string, std::function<void()>>> alerts;
    std::future<void> monitoringTask;
    std::condition_variable cv;
    std::mutex cv_m;
    std::string alertNotificationMethod;

    void monitoringLoop();
    void log(const std::string &message) const;
    void handleError(const std::string &error) const;
    void checkAlerts();
    void sendNotification(const std::string &message) const;
    void sendEmail(const std::string &message) const;
    void sendSMS(const std::string &message) const;
    void sendToMessenger(const std::string &message) const;
};

#endif // MONITORINGSERVICE_H




