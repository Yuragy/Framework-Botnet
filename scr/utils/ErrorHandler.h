#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>
#include <functional>
#include <vector>

class ErrorHandler {
public:
    static void handleError(const std::string &error);
    static void addCustomHandler(const std::function<void(const std::string&)>& handler);
    static void setLogLevel(Logger::Level level);

private:
    static std::vector<std::function<void(const std::string&)>> customHandlers;
};

#endif // ERRORHANDLER_H

