#include "Metrics.h"
#include "Logger.h"
#include <chrono>
#include <functional>

std::unordered_map<std::string, std::atomic<int>> Metrics::counters;
std::unordered_map<std::string, std::atomic<double>> Metrics::gauges;
std::unordered_map<std::string, std::vector<double>> Metrics::histograms;
std::shared_mutex Metrics::metricsMutex;

void Metrics::incrementCounter(const std::string &name) {
    {
        std::unique_lock lock(metricsMutex);
        ++counters[name];
    }
    logMetric("Incrementing counter", name, counters[name].load());
}

void Metrics::updateGauge(const std::string &name, double value) {
    {
        std::unique_lock lock(metricsMutex);
        gauges[name] = value;
    }
    logMetric("Updating gauge", name, value);
}

int Metrics::getCounterValue(const std::string &name) {
    std::shared_lock lock(metricsMutex);
    return counters[name].load();
}

double Metrics::getGaugeValue(const std::string &name) {
    std::shared_lock lock(metricsMutex);
    return gauges[name].load();
}

void Metrics::updateHistogram(const std::string &name, double value) {
    {
        std::unique_lock lock(metricsMutex);
        histograms[name].push_back(value);
    }
    logMetric("Updating histogram", name, value);
}

std::vector<double> Metrics::getHistogram(const std::string &name) {
    std::shared_lock lock(metricsMutex);
    return histograms[name];
}

void Metrics::timeExecution(const std::string &name, const std::function<void()>& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    updateHistogram(name, duration.count());
}

void Metrics::logMetric(const std::string &action, const std::string &name, double value) {
    Logger::log(Logger::INFO, action + ": " + name + " to value: " + std::to_string(value));
}




