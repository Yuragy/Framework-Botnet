#ifndef PHISHINGMANAGER_H
#define PHISHINGMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <future>
#include <memory>

class PhishingManager {
public:
    PhishingManager();
    ~PhishingManager();

    void sendPhishingEmail(const std::string &recipient, const std::string &phishingLink, bool isHtml = false);
    void sendPhishingSMS(const std::string &recipient, const std::string &phishingLink);
    void sendPhishingTelegramMessage(const std::string &recipient, const std::string &phishingLink);
    void sendPhishingWhatsAppMessage(const std::string &recipient, const std::string &phishingLink);
    void sendPhishingFacebookMessage(const std::string &recipient, const std::string &phishingLink);
    void sendPhishingInstagramMessage(const std::string &recipient, const std::string &phishingLink);

    void sendPhishingEmails(const std::vector<std::string> &recipients, const std::string &phishingLink, bool isHtml = false);
    void sendPhishingSMSs(const std::vector<std::string> &recipients, const std::string &phishingLink);
    void sendPhishingTelegramMessages(const std::vector<std::string> &recipients, const std::string &phishingLink);
    void sendPhishingWhatsAppMessages(const std::vector<std::string> &recipients, const std::string &phishingLink);
    void sendPhishingFacebookMessages(const std::vector<std::string> &recipients, const std::string &phishingLink);
    void sendPhishingInstagramMessages(const std::vector<std::string> &recipients, const std::string &phishingLink);

    void sendPhishingTemplateEmail(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders, bool isHtml = false);
    void sendPhishingTemplateSMS(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders);
    void sendPhishingTemplateTelegramMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders);
    void sendPhishingTemplateWhatsAppMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders);
    void sendPhishingTemplateFacebookMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders);
    void sendPhishingTemplateInstagramMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders);

    std::future<void> sendPhishingEmailAsync(const std::string &recipient, const std::string &phishingLink, bool isHtml = false);
    std::future<void> sendPhishingSMSAsync(const std::string &recipient, const std::string &phishingLink);
    std::future<void> sendPhishingTelegramMessageAsync(const std::string &recipient, const std::string &phishingLink);
    std::future<void> sendPhishingWhatsAppMessageAsync(const std::string &recipient, const std::string &phishingLink);
    std::future<void> sendPhishingFacebookMessageAsync(const std::string &recipient, const std::string &phishingLink);
    std::future<void> sendPhishingInstagramMessageAsync(const std::string &recipient, const std::string &phishingLink);

    void sendMalwareAttachmentEmail(const std::string &recipient, const std::string &payloadPath, bool isHtml = false);

    void setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader);

private:
    std::shared_ptr<TemplateLoader> templateLoader;

    std::string applyTemplate(const std::string &templateContent, const std::map<std::string, std::string> &placeholders);
    void logMessage(const std::string &recipient, const std::string &message, const std::string &status);
    std::string fetchUrlReputation(const std::string &url);
    bool validateUrl(const std::string &url);
    void handleSendError(const std::string &error);

    template<typename Sender>
    void sendPhishingMessage(Sender& sender, const std::string &recipient, const std::string &message);
};

#endif // PHISHINGMANAGER_H





