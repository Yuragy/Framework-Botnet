#ifndef METRICS_H
#define METRICS_H

#include <string>
#include <unordered_map>
#include <atomic>
#include <shared_mutex>
#include <vector>
#include <functional>

class Metrics {
public:
    static void incrementCounter(const std::string &name);
    static void updateGauge(const std::string &name, double value);
    static int getCounterValue(const std::string &name);
    static double getGaugeValue(const std::string &name);
    static void updateHistogram(const std::string &name, double value);
    static std::vector<double> getHistogram(const std::string &name);
    static void timeExecution(const std::string &name, const std::function<void()>& func);

private:
    static std::unordered_map<std::string, std::atomic<int>> counters;
    static std::unordered_map<std::string, std::atomic<double>> gauges;
    static std::unordered_map<std::string, std::vector<double>> histograms;
    static std::shared_mutex metricsMutex;
    
    static void logMetric(const std::string &action, const std::string &name, double value);
};

#endif // METRICS_H




