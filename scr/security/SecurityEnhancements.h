#ifndef SECURITYENHANCEMENTS_H
#define SECURITYENHANCEMENTS_H

class SecurityEnhancements {
public:
    static void apply();

private:
    static void applyMemoryProtection();
    static void applyNetworkSecurity();
    static void applyFileIntegrityChecks();
    static void applyProcessIsolation();
    static void logAction(const std::string &action);
    static std::string calculateFileHash(const std::string &filePath);
    static bool configureFirewallRule(const std::string &rule, bool enable);
    static bool createGuardPage(void *address, size_t size);
    static bool isPageGuarded(void *address);
};

#endif // SECURITYENHANCEMENTS_H

