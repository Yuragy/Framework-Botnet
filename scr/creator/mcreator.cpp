#include "MCreator.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>

std::string MCreator::encryptionKey = "defaultkey123456"; 
std::string MCreator::obfuscationPattern = "~"; 
int MCreator::logLevel = 1; 
std::string MCreator::notificationEndpoint = "";

std::string MCreator::createMalware(const std::string &payloadPath, const std::string &extension) {
    std::string payload = readPayloadFromFile(payloadPath);
    std::string obfuscatedPayload = obfuscatePayload(payload);
    std::string encryptedPayload = encryptPayload(obfuscatedPayload);
    std::string filePath = generateUniqueFileName(extension);

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to create malware file");
    }
    outFile << encryptedPayload;
    outFile.close();

    logCreation(filePath);
    notifyCreation("success", filePath);
    return filePath;
}

void MCreator::setEncryptionKey(const std::string &key) {
    encryptionKey = key;
}

void MCreator::setObfuscationPattern(const std::string &pattern) {
    obfuscationPattern = pattern;
}

void MCreator::setLogLevel(int level) {
    logLevel = level;
}

void MCreator::setNotificationEndpoint(const std::string &endpoint) {
    notificationEndpoint = endpoint;
}

std::string MCreator::createWindowsMalware(const std::string &payloadPath) {
    return createMalware(payloadPath, ".exe");
}

std::string MCreator::createLinuxMalware(const std::string &payloadPath) {
    return createMalware(payloadPath, ".elf");
}

std::string MCreator::createMacOSMalware(const std::string &payloadPath) {
    return createMalware(payloadPath, ".dmg");
}

std::string MCreator::createAndroidMalware(const std::string &payloadPath) {
    return createMalware(payloadPath, ".apk");
}

std::string MCreator::createiOSMalware(const std::string &payloadPath) {
    return createMalware(payloadPath, ".ipa");
}

std::string MCreator::generateUniqueFileName(const std::string &extension) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
    return "malware_" + ss.str() + extension;
}

std::string MCreator::readPayloadFromFile(const std::string &filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open payload file");
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    return buffer.str();
}

std::string MCreator::encryptPayload(const std::string &payload) {
    std::string key = encryptionKey; 
    std::string iv = "exampleiv1234567";

    std::string encryptedPayload;
    encryptedPayload.resize(payload.size() + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    int len;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize AES encryption");
    }

    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&encryptedPayload[0]), &len, reinterpret_cast<const unsigned char*>(payload.c_str()), payload.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to update AES encryption");
    }

    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&encryptedPayload[0]) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize AES encryption");
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    encryptedPayload.resize(ciphertext_len);

    return encryptedPayload;
}

std::string MCreator::obfuscatePayload(const std::string &payload) {
    std::string obfuscatedPayload = payload;
    for (char &c : obfuscatedPayload) {
        c = obfuscationPattern[0];
    }
    return obfuscatedPayload;
}

void MCreator::logCreation(const std::string &filePath) {
    if (logLevel >= 1) {
        std::cout << "Malware created at: " << filePath << std::endl;
    }
}

void MCreator::notifyCreation(const std::string &status, const std::string &filePath) {
    if (!notificationEndpoint.empty()) {
        // Логика отправки уведомления на указанный endpoint
        std::cout << "Notifying creation status: " << status << " for file: " << filePath << std::endl;
    }
}




