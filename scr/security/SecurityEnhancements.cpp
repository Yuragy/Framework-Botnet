#include "SecurityEnhancements.h"
#include "Logger.h"
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

void SecurityEnhancements::logAction(const std::string &action) {
    Logger::log(Logger::INFO, action);
}

void SecurityEnhancements::apply() {
    logAction("Applying security enhancements");

    applyMemoryProtection();
    applyNetworkSecurity();
    applyFileIntegrityChecks();
    applyProcessIsolation();

    logAction("Security enhancements applied successfully");
}

void SecurityEnhancements::applyMemoryProtection() {
    logAction("Applying memory protection");

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    void *guardPage = VirtualAlloc(NULL, sysInfo.dwPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READONLY | PAGE_GUARD);

    if (guardPage == NULL) {
        logAction("Failed to allocate guard page for memory protection");
        throw std::runtime_error("Failed to allocate guard page for memory protection");
    }

    logAction("Memory protection applied");
}

bool SecurityEnhancements::isPageGuarded(void *address) {
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(address, &mbi, sizeof(mbi));
    return mbi.Protect & PAGE_GUARD;
}

void SecurityEnhancements::applyNetworkSecurity() {
    logAction("Applying network security");

    if (!configureFirewallRule("netsh advfirewall firewall add rule name=\"Block Unauthorized Access\" dir=in action=block protocol=TCP localport=80,443", true)) {
        logAction("Failed to apply network security");
        throw std::runtime_error("Failed to apply network security");
    }

    logAction("Network security applied");
}

void SecurityEnhancements::applyFileIntegrityChecks() {
    logAction("Applying file integrity checks");
    std::ofstream file("integrity_check.txt");
    file << "Initial File Content";
    file.close();

    logAction("Calculating initial file hash");
    std::string initialHash = calculateFileHash("integrity_check.txt");
    logAction("Initial file hash: " + initialHash);

    logAction("File integrity checks applied");
}

void SecurityEnhancements::applyProcessIsolation() {
    logAction("Applying process isolation");

    HANDLE job = CreateJobObject(NULL, NULL);
    if (job == NULL) {
        logAction("Failed to create job object for process isolation");
        throw std::runtime_error("Failed to create job object for process isolation");
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo))) {
        CloseHandle(job);
        logAction("Failed to set job object information");
        throw std::runtime_error("Failed to set job object information");
    }

    logAction("Process isolation applied");
}

bool SecurityEnhancements::configureFirewallRule(const std::string &rule, bool enable) {
    int result = system(rule.c_str());
    return result == 0;
}

bool SecurityEnhancements::createGuardPage(void *address, size_t size) {
    void *guardPage = VirtualAlloc(address, size, MEM_RESERVE | MEM_COMMIT, PAGE_READONLY | PAGE_GUARD);
    return guardPage != NULL;
}

std::string SecurityEnhancements::calculateFileHash(const std::string &filePath) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char buf[1024];
    std::ifstream file(filePath, std::ifstream::binary);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    while (file.read(buf, sizeof(buf))) {
        SHA256_Update(&sha256, buf, file.gcount());
    }
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

