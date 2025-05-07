#include "LinuxUtils.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <array>
#include <memory>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <filesystem>

namespace fs = std::filesystem;

void LinuxUtils::performLinuxSpecificTask() {
    Logger::log(Logger::INFO, "Executing a Linux-specific task.");
}

std::string LinuxUtils::executeCommand(const std::string &command) {
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

std::string LinuxUtils::getKernelVersion() {
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        throw std::runtime_error("uname() failed!");
    }
    return buffer.release;
}

void LinuxUtils::optimizePerformance() {
    Logger::log(Logger::INFO, "Optimizing system performance.");
    executeCommand("systemctl stop some_unused_service");
    executeCommand("sysctl -w vm.swappiness=10");
}

void LinuxUtils::ensureSecurity() {
    Logger::log(Logger::INFO, "Applying security measures.");
    executeCommand("ufw enable");
    executeCommand("apt-get update && apt-get upgrade -y");
}

bool LinuxUtils::checkRootPrivileges() {
    if (geteuid() != 0) {
        std::cerr << "Root privileges are required for this operation." << std::endl;
        return false;
    }
    return true;
}

void LinuxUtils::configureFirewall(const std::string &rule) {
    if (!checkRootPrivileges()) {
        throw std::runtime_error("Root privileges are required for this operation.");
    }
    std::string command = "ufw " + rule;
    Logger::log(Logger::INFO, "Configuring firewall with rule: " + rule);
    executeCommand(command);
}

std::string LinuxUtils::getSystemLoad() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        throw std::runtime_error("sysinfo() failed!");
    }
    std::ostringstream oss;
    oss << "1 minute load: " << info.loads[0] / 65536.0 << ", "
        << "5 minute load: " << info.loads[1] / 65536.0 << ", "
        << "15 minute load: " << info.loads[2] / 65536.0;
    return oss.str();
}

void LinuxUtils::createBackup(const std::string &source, const std::string &destination) {
    if (!checkRootPrivileges()) {
        throw std::runtime_error("Root privileges are required for this operation.");
    }
    try {
        Logger::log(Logger::INFO, "Creating backup from " + source + " to " + destination);
        fs::copy(source, destination, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error &e) {
        Logger::log(Logger::ERROR, std::string("Error creating backup: ") + e.what());
        throw;
    }
}

void LinuxUtils::updateSystem() {
    Logger::log(Logger::INFO, "Updating system packages.");
    executeCommand("apt-get update && apt-get upgrade -y");
}

void LinuxUtils::monitorDiskUsage() {
    Logger::log(Logger::INFO, "Monitoring disk usage.");
    std::string result = executeCommand("df -h");
    std::cout << result << std::endl;
}

void LinuxUtils::configureNetwork(const std::string &interface, const std::string &ip, const std::string &netmask, const std::string &gateway) {
    if (!checkRootPrivileges()) {
        throw std::runtime_error("Root privileges are required for this operation.");
    }
    Logger::log(Logger::INFO, "Configuring network interface: " + interface);
    executeCommand("ifconfig " + interface + " " + ip + " netmask " + netmask);
    executeCommand("route add default gw " + gateway + " " + interface);
}


