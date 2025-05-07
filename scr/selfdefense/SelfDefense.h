#ifndef SELFDEFENSE_H
#define SELFDEFENSE_H

#include <string>

class SelfDefense {
public:
    static void activate();
    static void deactivate();
    static bool isActive();
    static int getProtectionLevel();

private:
    static void setProtectionLevel(int level);
    static void logOperation(const std::string& operation, bool success, const std::string& additionalInfo = "");
    static bool active;
    static int protectionLevel;
    static void applyAdvancedProtection();
    static bool detectSandbox();
    static bool detectAnalysisTools();
    static void monitorSystemChanges();
    static void detectAndPreventRootkits();
    static bool detectAntivirus();
};

#endif // SELFDEFENSE_H


