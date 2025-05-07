#include "WinAPIUtils.h"
#include <windows.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>
#include <array>
#include <memory>
#include <atlbase.h>
#include <Wbemidl.h>
#include <comdef.h>

#pragma comment(lib, "wbemuuid.lib")

std::string WinAPIUtils::executeCommand(const std::string &command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string WinAPIUtils::getOSVersion() {
    std::ostringstream os;
    OSVERSIONINFOEX info;
    ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&info);
    os << "Windows " << info.dwMajorVersion << "." << info.dwMinorVersion;
    return os.str();
}

void WinAPIUtils::optimizePerformance() {
    std::cout << "Optimizing performance..." << std::endl;
    executeCommand("powercfg -change -monitor-timeout-ac 5");
    executeCommand("powercfg -change -monitor-timeout-dc 5");
}

void WinAPIUtils::ensureSecurity() {
    std::cout << "Ensuring security..." << std::endl;
    executeCommand("netsh advfirewall set allprofiles state on");
    executeCommand("wuauclt /detectnow /updatenow");
}

bool WinAPIUtils::checkAdminPrivileges() {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup);
    CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
    FreeSid(administratorsGroup);
    if (!isAdmin) {
        std::cerr << "This operation requires administrator privileges." << std::endl;
    }
    return isAdmin;
}

void WinAPIUtils::configureFirewall(const std::string &rule) {
    if (!checkAdminPrivileges()) {
        throw std::runtime_error("Operation requires administrator privileges");
    }
    std::string command = "netsh advfirewall firewall " + rule;
    std::cout << "Configuring firewall with rule: " << rule << std::endl;
    executeCommand(command);
}

void WinAPIUtils::setFilePermissions(const std::string &filePath, const std::string &permissions) {
    std::string command = "icacls " + filePath + " /grant " + permissions;
    std::cout << "Setting file permissions: " << permissions << " for file: " << filePath << std::endl;
    executeCommand(command);
}

void WinAPIUtils::scheduleTask(const std::string &taskName, const std::string &command, const std::string &time) {
    std::string taskCommand = "schtasks /create /tn " + taskName + " /tr \"" + command + "\" /sc once /st " + time;
    std::cout << "Scheduling task: " << taskName << " with command: " << command << " at time: " << time << std::endl;
    executeCommand(taskCommand);
}

std::string WinAPIUtils::readSystemLog(const std::string &logType) {
    std::string command = "wevtutil qe " + logType + " /f:text /rd:true /c:1";
    std::cout << "Reading system log for type: " << logType << std::endl;
    return executeCommand(command);
}

void WinAPIUtils::manageStartupPrograms(const std::string &program, bool add) {
    std::string command = add ? "reg add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run /v " + program + " /t REG_SZ /d \"" + program + "\""
                              : "reg delete HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run /v " + program + " /f";
    std::cout << (add ? "Adding " : "Removing ") << program << " to/from startup programs." << std::endl;
    executeCommand(command);
}

bool WinAPIUtils::isProcessRunning(const std::string &processName) {
    std::string command = "tasklist /FI \"IMAGENAME eq " + processName + "\"";
    std::string result = executeCommand(command);
    return result.find(processName) != std::string::npos;
}

