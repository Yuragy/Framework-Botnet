#include "MacOSUtils.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <array>
#include <memory>
#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <future>

void MacOSUtils::performMacOSSpecificTask() {
    Logger::log(Logger::INFO, "Executing a MacOS-specific task.");
}

std::string MacOSUtils::executeCommand(const std::string &command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string MacOSUtils::getOSVersion() {
    std::string version;
    CFPropertyListRef plist;
    CFStringRef versionKey = CFStringCreateWithCString(NULL, "ProductVersion", kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("/System/Library/CoreServices/SystemVersion.plist"), kCFURLPOSIXPathStyle, false);
    
    if (url) {
        CFReadStreamRef stream = CFReadStreamCreateWithFile(kCFAllocatorDefault, url);
        if (stream) {
            if (CFReadStreamOpen(stream)) {
                plist = CFPropertyListCreateWithStream(kCFAllocatorDefault, stream, 0, kCFPropertyListImmutable, NULL, NULL);
                if (plist) {
                    CFDictionaryRef dict = (CFDictionaryRef)plist;
                    CFStringRef ver = (CFStringRef)CFDictionaryGetValue(dict, versionKey);
                    const char *cStr = CFStringGetCStringPtr(ver, kCFStringEncodingUTF8);
                    version = std::string(cStr);
                    CFRelease(plist);
                }
                CFReadStreamClose(stream);
            }
            CFRelease(stream);
        }
        CFRelease(url);
    }
    CFRelease(versionKey);
    return version;
}

void MacOSUtils::optimizePerformance() {
    Logger::log(Logger::INFO, "Optimizing performance.");
    executeCommand("launchctl unload /System/Library/LaunchAgents/com.apple.imagent.plist");
    executeCommand("sudo sysctl -w kern.maxfiles=20480");
}

void MacOSUtils::ensureSecurity() {
    Logger::log(Logger::INFO, "Ensuring security.");
    executeCommand("sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setglobalstate on");
    executeCommand("softwareupdate --install --all");
}

bool MacOSUtils::checkRootPrivileges() {
    if (geteuid() != 0) {
        std::cerr << "This operation requires root privileges." << std::endl;
        return false;
    }
    return true;
}

void MacOSUtils::configureFirewall(const std::string &rule) {
    if (!checkRootPrivileges()) {
        throw std::runtime_error("Operation requires root privileges");
    }
    std::string command = "sudo /usr/libexec/ApplicationFirewall/socketfilterfw " + rule;
    Logger::log(Logger::INFO, "Configuring firewall with rule: " + rule);
    executeCommand(command);
}

void MacOSUtils::setFilePermissions(const std::string &filePath, const std::string &permissions) {
    std::string command = "chmod " + permissions + " " + filePath;
    Logger::log(Logger::INFO, "Setting file permissions: " + permissions + " for file: " + filePath);
    executeCommand(command);
}

void MacOSUtils::scheduleTask(const std::string &taskName, const std::string &command, const std::string &time) {
    std::string cronCommand = "(crontab -l 2>/dev/null; echo \"" + time + " " + command + "\") | crontab -";
    Logger::log(Logger::INFO, "Scheduling task: " + taskName + " with command: " + command + " at time: " + time);
    executeCommand(cronCommand);
}

std::string MacOSUtils::readSystemLog(const std::string &logType) {
    std::string command = "log show --predicate 'process == \"" + logType + "\"' --info";
    Logger::log(Logger::INFO, "Reading system log for type: " + logType);
    return executeCommand(command);
}

void MacOSUtils::manageStartupPrograms(const std::string &program, bool add) {
    std::string command = add ? "osascript -e 'tell application \"System Events\" to make new login item at end with properties {path:\"" + program + "\", hidden:false}'" 
                              : "osascript -e 'tell application \"System Events\" to delete login item \"" + program + "\"'";
    Logger::log(Logger::INFO, (add ? "Adding " : "Removing ") + program + " to/from startup programs.");
    executeCommand(command);
}

bool MacOSUtils::isProcessRunning(const std::string &processName) {
    std::string command = "pgrep " + processName;
    std::string result = executeCommand(command);
    return !result.empty();
}

std::future<std::string> MacOSUtils::executeCommandAsync(const std::string &command) {
    return std::async(std::launch::async, [command]() {
        return executeCommand(command);
    });
}

std::future<std::string> MacOSUtils::getOSVersionAsync() {
    return std::async(std::launch::async, []() {
        return getOSVersion();
    });
}

std::future<void> MacOSUtils::optimizePerformanceAsync() {
    return std::async(std::launch::async, []() {
        optimizePerformance();
    });
}

std::future<void> MacOSUtils::ensureSecurityAsync() {
    return std::async(std::launch::async, []() {
        ensureSecurity();
    });
}

std::future<bool> MacOSUtils::checkRootPrivilegesAsync() {
    return std::async(std::launch::async, []() {
        return checkRootPrivileges();
    });
}

std::future<void> MacOSUtils::configureFirewallAsync(const std::string &rule) {
    return std::async(std::launch::async, [rule]() {
        configureFirewall(rule);
    });
}

std::future<void> MacOSUtils::setFilePermissionsAsync(const std::string &filePath, const std::string &permissions) {
    return std::async(std::launch::async, [filePath, permissions]() {
        setFilePermissions(filePath, permissions);
    });
}

std::future<void> MacOSUtils::scheduleTaskAsync(const std::string &taskName, const std::string &command, const std::string &time) {
    return std::async(std::launch::async, [taskName, command, time]() {
        scheduleTask(taskName, command, time);
    });
}

std::future<std::string> MacOSUtils::readSystemLogAsync(const std::string &logType) {
    return std::async(std::launch::async, [logType]() {
        return readSystemLog(logType);
    });
}

std::future<void> MacOSUtils::manageStartupProgramsAsync(const std::string &program, bool add) {
    return std::async(std::launch::async, [program, add]() {
        manageStartupPrograms(program, add);
    });
}

std::future<bool> MacOSUtils::isProcessRunningAsync(const std::string &processName) {
    return std::async(std::launch::async, [processName]() {
        return isProcessRunning(processName);
    });
}


