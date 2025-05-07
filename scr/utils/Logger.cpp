#include "Logger.h"

// Initialize the static members
std::shared_ptr<spdlog::logger> Logger::logger = nullptr;
std::map<std::string, int> Logger::metrics;
std::mutex Logger::metricsMutex;

void Logger::init(const std::string& logFilePath, Level logLevel) {
    try {
        logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_logger", logFilePath);
        spdlog::set_default_logger(logger);
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        setLevel(logLevel);
        Logger::log(INFO, "Logger initialized with file: " + logFilePath);
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::log(Level level, const std::string &message) {
    try {
        ensureLoggerInitialized();
        logger->log(toSpdlogLevel(level), message);
    } catch (const std::exception &e) {
        std::cerr << "Logging failed: " << e.what() << std::endl;
    }
}

void Logger::logWithParams(Level level, const std::string &message, const std::map<std::string, std::string> &params) {
    try {
        ensureLoggerInitialized();
        std::string formattedMessage = message;
        for (const auto &param : params) {
            formattedMessage += " [" + param.first + "=" + param.second + "]";
        }
        logger->log(toSpdlogLevel(level), formattedMessage);
    } catch (const std::exception &e) {
        std::cerr << "Logging with params failed: " << e.what() << std::endl;
    }
}

void Logger::setLevel(Level level) {
    try {
        ensureLoggerInitialized();
        logger->set_level(toSpdlogLevel(level));
        Logger::log(INFO, "Log level set to: " + std::to_string(level));
    } catch (const std::exception &e) {
        std::cerr << "Setting log level failed: " << e.what() << std::endl;
    }
}

spdlog::level::level_enum Logger::toSpdlogLevel(Level level) {
    switch (level) {
        case TRACE: return spdlog::level::trace;
        case DEBUG: return spdlog::level::debug;
        case INFO: return spdlog::level::info;
        case WARNING: return spdlog::level::warn;
        case ERROR: return spdlog::level::err;
        case FATAL: return spdlog::level::critical;
        default: return spdlog::level::info;
    }
}

void Logger::ensureLoggerInitialized() {
    if (!logger) {
        init();
    }
}

void Logger::logMetric(const std::string &metricName, int value) {
    try {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics[metricName] += value;
        log(INFO, "Metric: " + metricName + " = " + std::to_string(metrics[metricName]));
    } catch (const std::exception &e) {
        std::cerr << "Logging metric failed: " << e.what() << std::endl;
    }
}








