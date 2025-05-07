#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <map>
#include <mutex>

class Logger {
public:
    enum Level { TRACE, DEBUG, INFO, WARNING, ERROR, FATAL };

    static void init(const std::string& logFilePath = "logs/async_log.txt", Level logLevel = INFO);
    static void log(Level level, const std::string &message);
    static void logWithParams(Level level, const std::string &message, const std::map<std::string, std::string> &params);
    static void setLevel(Level level);
    static void logMetric(const std::string &metricName, int value);

private:
    static std::shared_ptr<spdlog::logger> logger;
    static std::map<std::string, int> metrics;
    static std::mutex metricsMutex;
    static spdlog::level::level_enum toSpdlogLevel(Level level);
    static void ensureLoggerInitialized();
};

#endif // LOGGER_H








