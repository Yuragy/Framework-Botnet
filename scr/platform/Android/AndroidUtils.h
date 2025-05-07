#ifndef ANDROIDUTILS_H
#define ANDROIDUTILS_H

#include <string>
#include <vector>
#include <future>
#include <map>

namespace AndroidUtils {
    void performAndroidSpecificTask();
    std::vector<std::string> getInstalledApps();
    bool checkAppInstalled(const std::string& packageName);
    void launchApp(const std::string& packageName);
    void uninstallApp(const std::string& packageName);
    
    std::map<std::string, std::string> getSystemInfo();
    
    std::future<std::vector<std::string>> getInstalledAppsAsync();
    std::future<bool> checkAppInstalledAsync(const std::string& packageName);
    std::future<void> launchAppAsync(const std::string& packageName);
    std::future<void> uninstallAppAsync(const std::string& packageName);
}

#endif // ANDROIDUTILS_H


