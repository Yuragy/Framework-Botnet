#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include "MessageSender.h"
#include "MCreator.h" 
#include "TemplateLoader.h" 
#include "Logger.h"
#include "URLShortener.h"
#include <string>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include <json/json.h>
#include <map>

class EmailSender : public MessageSender {
public:
    EmailSender(const std::string &configFilePath);
    void sendMessage(const std::string &recipient, const std::string &message) override;
    void sendMessage(const std::vector<std::string> &recipients, const std::string &message);
    void sendMessageWithAttachment(const std::vector<std::string> &recipients, const std::string &message, const std::vector<std::string> &filePaths);
    void sendEncryptedMessage(const std::vector<std::string> &recipients, const std::string &message, const std::string &encryptionKey);
    void getDeliveryReports();
    void sendTemplateMessage(const std::vector<std::string> &recipients, const std::string &templateName, const std::map<std::string, std::string> &placeholders);
    void sendHTMLMessage(const std::vector<std::string> &recipients, const std::string &htmlMessage);
    void sendMalware(const std::vector<std::string> &recipients, const std::string &payloadPath);

private:
    std::string smtpServer;
    int port;
    std::string username;
    std::string password;
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl;

    void configureSMTP();
    void handleErrors(const std::string &error);
    void logDetails(const std::string &stage, const std::string &details);
    void sendEmailBatch(const std::vector<std::string> &batch, const std::string &message, const std::vector<std::string> &filePaths = {});
    void loadConfig(const std::string &configFilePath);
    std::string applyTemplate(const std::string &templateContent, const std::map<std::string, std::string> &placeholders);
    void retrySendEmailBatch(const std::vector<std::string> &batch, const std::string &message, const std::vector<std::string> &filePaths);
    std::string encryptMessage(const std::string &message, const std::string &encryptionKey);
};

#endif // EMAILSENDER_H




