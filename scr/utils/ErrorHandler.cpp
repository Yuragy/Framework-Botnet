#include "ErrorHandler.h"
#include "Logger.h"

std::vector<std::function<void(const std::string&)>> ErrorHandler::customHandlers;

void ErrorHandler::handleError(const std::string &error) {
    Logger::log(Logger::ERROR, "Error: " + error);

    for (const auto& handler : customHandlers) {
        handler(error);
    }
}

void ErrorHandler::addCustomHandler(const std::function<void(const std::string&)>& handler) {
    customHandlers.push_back(handler);
}

void ErrorHandler::setLogLevel(Logger::Level level) {
    Logger::setLevel(level);
}

