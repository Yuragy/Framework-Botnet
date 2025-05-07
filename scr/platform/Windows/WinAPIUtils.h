#ifndef WINAPIUTILS_H
#define WINAPIUTILS_H

#include <string>

namespace WinAPIUtils {
    std::string executeCommand(const std::string &command);
    std::string getOSVersion();
    void optimizePerformance();
    void ensureSecurity();
    bool checkAdminPrivileges();
    void configureFirewall(const std::string &rule);
    void setFilePermissions(const std::string &filePath, const std::string &permissions);
    void scheduleTask(const std::string &taskName, const std::string &command, const std::string &time);
    std::string readSystemLog(const std::string &logType);
    void manageStartupPrograms(const std::string &program, bool add);
    bool isProcessRunning(const std::string &processName);
}

#endif // WINAPIUTILS_H

