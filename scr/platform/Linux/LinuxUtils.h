#ifndef LINUXUTILS_H
#define LINUXUTILS_H

#include <string>

namespace LinuxUtils {
    void performLinuxSpecificTask();
    std::string executeCommand(const std::string &command);
    std::string getKernelVersion();
    void optimizePerformance();
    void ensureSecurity();
    bool checkRootPrivileges();
    void configureFirewall(const std::string &rule);
    std::string getSystemLoad();
    void createBackup(const std::string &source, const std::string &destination);
    void updateSystem();
    void monitorDiskUsage();
    void configureNetwork(const std::string &interface, const std::string &ip, const std::string &netmask, const std::string &gateway);
}

#endif // LINUXUTILS_H


