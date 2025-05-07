#ifndef MACOSUTILS_H
#define MACOSUTILS_H

#include <string>
#include <future>

namespace MacOSUtils {
    void performMacOSSpecificTask();
    std::string executeCommand(const std::string &command);
    std::string getOSVersion();
    void optimizePerformance();
    void ensureSecurity();
    bool checkRootPrivileges();
    void configureFirewall(const std::string &rule);
    void setFilePermissions(const std::string &filePath, const std::string &permissions);
    void scheduleTask(const std::string &taskName, const std::string &command, const std::string &time);
    std::string readSystemLog(const std::string &logType);
    void manageStartupPrograms(const std::string &program, bool add);
    bool isProcessRunning(const std::string &processName);

    std::future<std::string> executeCommandAsync(const std::string &command);
    std::future<std::string> getOSVersionAsync();
    std::future<void> optimizePerformanceAsync();
    std::future<void> ensureSecurityAsync();
    std::future<bool> checkRootPrivilegesAsync();
    std::future<void> configureFirewallAsync(const std::string &rule);
    std::future<void> setFilePermissionsAsync(const std::string &filePath, const std::string &permissions);
    std::future<void> scheduleTaskAsync(const std::string &taskName, const std::string &command, const std::string &time);
    std::future<std::string> readSystemLogAsync(const std::string &logType);
    std::future<void> manageStartupProgramsAsync(const std::string &program, bool add);
    std::future<bool> isProcessRunningAsync(const std::string &processName);
}

#endif // MACOSUTILS_H


