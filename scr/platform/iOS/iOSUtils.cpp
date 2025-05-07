#include "iOSUtils.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <future>
#include <sstream>

namespace {
    std::string executeCommand(const std::string& command) {
        char buffer[128];
        std::string result = "";
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
                result += buffer;
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);
        return result;
    }
}

void iOSUtils::performiOSSpecificTask() {
    Logger::log(Logger::INFO, "Performing iOS specific task");
    std::cout << "Hello, iOS!" << std::endl;
}

std::vector<std::string> iOSUtils::getInstalledApps() {
    Logger::log(Logger::INFO, "Fetching list of installed apps");
    std::vector<std::string> apps;
    try {
        std::string result = executeCommand("ideviceinstaller -l");
        std::istringstream stream(result);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.find("CFBundleIdentifier") != std::string::npos) {
                std::string app = line.substr(line.find(":") + 2);
                apps.push_back(app);
            }
        }
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error fetching installed apps: ") + e.what());
    }
    return apps;
}

bool iOSUtils::checkAppInstalled(const std::string& bundleId) {
    Logger::log(Logger::INFO, "Checking if app is installed: " + bundleId);
    try {
        std::string result = executeCommand("ideviceinstaller -l | grep " + bundleId);
        return !result.empty();
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error checking app installation: ") + e.what());
        return false;
    }
}

void iOSUtils::launchApp(const std::string& bundleId) {
    Logger::log(Logger::INFO, "Launching app: " + bundleId);
    try {
        std::string result = executeCommand("idevicedebug run " + bundleId);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error launching app: ") + e.what());
    }
}

std::string iOSUtils::getBatteryStatus() {
    Logger::log(Logger::INFO, "Fetching battery status");
    try {
        return executeCommand("ideviceinfo -q com.apple.mobile.battery");
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error fetching battery status: ") + e.what());
        return "Error";
    }
}

std::string iOSUtils::getSystemInfo() {
    Logger::log(Logger::INFO, "Fetching system information");
    try {
        return executeCommand("ideviceinfo");
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error fetching system information: ") + e.what());
        return "Error";
    }
}

void iOSUtils::restartDevice() {
    Logger::log(Logger::INFO, "Restarting device");
    try {
        executeCommand("idevicediagnostics restart");
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error restarting device: ") + e.what());
    }
}

std::future<std::vector<std::string>> iOSUtils::getInstalledAppsAsync() {
    return std::async(std::launch::async, []() {
        return getInstalledApps();
    });
}

std::future<bool> iOSUtils::checkAppInstalledAsync(const std::string& bundleId) {
    return std::async(std::launch::async, [bundleId]() {
        return checkAppInstalled(bundleId);
    });
}

std::future<void> iOSUtils::launchAppAsync(const std::string& bundleId) {
    return std::async(std::launch::async, [bundleId]() {
        launchApp(bundleId);
    });
}

std::future<std::string> iOSUtils::getBatteryStatusAsync() {
    return std::async(std::launch::async, []() {
        return getBatteryStatus();
    });
}

std::future<std::string> iOSUtils::getSystemInfoAsync() {
    return std::async(std::launch::async, []() {
        return getSystemInfo();
    });
}

std::future<void> iOSUtils::restartDeviceAsync() {
    return std::async(std::launch::async, []() {
        restartDevice();
    });
}


