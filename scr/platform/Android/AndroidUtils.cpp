#include "AndroidUtils.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <future>
#include <sstream>
#include <chrono>

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

void AndroidUtils::performAndroidSpecificTask() {
    Logger::log(Logger::INFO, "Performing Android specific task");
    // Implementation of Android-specific task
}

std::vector<std::string> AndroidUtils::getInstalledApps() {
    auto start = std::chrono::high_resolution_clock::now();
    Logger::log(Logger::INFO, "Fetching list of installed apps");
    std::vector<std::string> apps;
    try {
        std::string result = executeCommand("pm list packages");
        std::istringstream stream(result);
        std::string line;
        while (std::getline(stream, line)) {
            apps.push_back(line.substr(8)); // Remove "package:" prefix
        }
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error fetching installed apps: ") + e.what());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    Logger::log(Logger::INFO, "Fetched list of installed apps in " + std::to_string(elapsed.count()) + " seconds");
    return apps;
}

bool AndroidUtils::checkAppInstalled(const std::string& packageName) {
    auto start = std::chrono::high_resolution_clock::now();
    Logger::log(Logger::INFO, "Checking if app is installed: " + packageName);
    try {
        std::string result = executeCommand("pm list packages " + packageName);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Logger::log(Logger::INFO, "Checked app installation in " + std::to_string(elapsed.count()) + " seconds");
        return !result.empty();
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error checking app installation: ") + e.what());
        return false;
    }
}

void AndroidUtils::launchApp(const std::string& packageName) {
    auto start = std::chrono::high_resolution_clock::now();
    Logger::log(Logger::INFO, "Launching app: " + packageName);
    try {
        std::string result = executeCommand("am start -n " + packageName + "/.MainActivity");
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error launching app: ") + e.what());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    Logger::log(Logger::INFO, "Launched app in " + std::to_string(elapsed.count()) + " seconds");
}

void AndroidUtils::uninstallApp(const std::string& packageName) {
    auto start = std::chrono::high_resolution_clock::now();
    Logger::log(Logger::INFO, "Uninstalling app: " + packageName);
    try {
        std::string result = executeCommand("pm uninstall " + packageName);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error uninstalling app: ") + e.what());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    Logger::log(Logger::INFO, "Uninstalled app in " + std::to_string(elapsed.count()) + " seconds");
}

std::map<std::string, std::string> AndroidUtils::getSystemInfo() {
    Logger::log(Logger::INFO, "Fetching system information");
    std::map<std::string, std::string> systemInfo;
    try {
        std::string result = executeCommand("getprop");
        std::istringstream stream(result);
        std::string line;
        while (std::getline(stream, line)) {
            auto pos = line.find(": ");
            if (pos != std::string::npos) {
                std::string key = line.substr(1, pos - 2);
                std::string value = line.substr(pos + 2, line.length() - pos - 3);
                systemInfo[key] = value;
            }
        }
    } catch (const std::exception& e) {
        Logger::log(Logger::ERROR, std::string("Error fetching system information: ") + e.what());
    }
    return systemInfo;
}

std::future<std::vector<std::string>> AndroidUtils::getInstalledAppsAsync() {
    return std::async(std::launch::async, []() {
        return getInstalledApps();
    });
}

std::future<bool> AndroidUtils::checkAppInstalledAsync(const std::string& packageName) {
    return std::async(std::launch::async, [packageName]() {
        return checkAppInstalled(packageName);
    });
}

std::future<void> AndroidUtils::launchAppAsync(const std::string& packageName) {
    return std::async(std::launch::async, [packageName]() {
        launchApp(packageName);
    });
}

std::future<void> AndroidUtils::uninstallAppAsync(const std::string& packageName) {
    return std::async(std::launch::async, [packageName]() {
        uninstallApp(packageName);
    });
}


