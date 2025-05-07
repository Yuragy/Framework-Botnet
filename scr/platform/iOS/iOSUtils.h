#ifndef IOSUTILS_H
#define IOSUTILS_H

#include <string>
#include <vector>
#include <future>

namespace iOSUtils {
    void performiOSSpecificTask();
    std::vector<std::string> getInstalledApps();
    bool checkAppInstalled(const std::string& bundleId);
    void launchApp(const std::string& bundleId);
    std::string getBatteryStatus();
    std::string getSystemInfo();
    void restartDevice();

    std::future<std::vector<std::string>> getInstalledAppsAsync();
    std::future<bool> checkAppInstalledAsync(const std::string& bundleId);
    std::future<void> launchAppAsync(const std::string& bundleId);
    std::future<std::string> getBatteryStatusAsync();
    std::future<std::string> getSystemInfoAsync();
    std::future<void> restartDeviceAsync();
}

#endif // IOSUTILS_H


