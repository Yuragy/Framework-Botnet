#include "TemplateLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>
#include <stdexcept>
#include "Logger.h"
#include "EncryptionUtils.h"

TemplateLoader::TemplateLoader() {
    // Constructor initialization
}

std::string TemplateLoader::loadTemplateFromFile(const std::string &filePath, const std::string &format) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logOperation("LoadTemplateFromFile", false, "Unable to open template file: " + filePath);
        throw std::runtime_error("Unable to open template file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    logOperation("LoadTemplateFromFile", true, "Loaded template from file: " + filePath);
    return buffer.str();
}

std::string TemplateLoader::loadTemplateFromDatabase(const std::string &templateName) {
    std::string templateContent = "Template content from database for: " + templateName;
    logOperation("LoadTemplateFromDatabase", true, "Loaded template from database: " + templateName);
    return templateContent;
}

std::future<std::string> TemplateLoader::loadTemplateFromFileAsync(const std::string &filePath, const std::string &format) {
    return std::async(std::launch::async, &TemplateLoader::loadTemplateFromFile, this, filePath, format);
}

std::future<std::string> TemplateLoader::loadTemplateFromDatabaseAsync(const std::string &templateName) {
    return std::async(std::launch::async, &TemplateLoader::loadTemplateFromDatabase, this, templateName);
}

std::string TemplateLoader::fillTemplate(const std::string &templateStr, const std::unordered_map<std::string, std::string> &data) {
    return replacePlaceholders(templateStr, data);
}

void TemplateLoader::cacheTemplate(const std::string &templateName, const std::string &templateContent, std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    templateCache[templateName] = {templateContent, std::chrono::system_clock::now() + ttl};
    logOperation("CacheTemplate", true, "Cached template: " + templateName);
}

std::string TemplateLoader::getTemplateFromCache(const std::string &templateName) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = templateCache.find(templateName);
    if (it != templateCache.end() && it->second.expiryTime > std::chrono::system_clock::now()) {
        logOperation("GetTemplateFromCache", true, "Template found in cache: " + templateName);
        return it->second.content;
    }
    logOperation("GetTemplateFromCache", false, "Template not found or expired in cache: " + templateName);
    throw std::runtime_error("Template not found or expired in cache: " + templateName);
}

std::string TemplateLoader::loadLocalizedTemplate(const std::string &templateName, const std::string &locale) {
    std::string localizedTemplateName = templateName + "_" + locale;
    return loadTemplateFromDatabase(localizedTemplateName);
}

std::string TemplateLoader::loadTemplateFromAPI(const std::string &apiUrl) {
    // Реализация загрузки шаблона с веб-сервиса по URL
    std::string apiResponse = "Template content from API for: " + apiUrl;
    logOperation("LoadTemplateFromAPI", true, "Loaded template from API: " + apiUrl);
    return apiResponse;
}

std::string TemplateLoader::replacePlaceholders(const std::string &templateStr, const std::unordered_map<std::string, std::string> &data) {
    std::string filledTemplate = templateStr;
    for (const auto &pair : data) {
        std::string placeholder = "{{" + pair.first + "}}";
        size_t pos = filledTemplate.find(placeholder);
        while (pos != std::string::npos) {
            filledTemplate.replace(pos, placeholder.length(), pair.second);
            pos = filledTemplate.find(placeholder, pos + pair.second.length());
        }
    }
    return filledTemplate;
}

void TemplateLoader::logOperation(const std::string &operation, bool success, const std::string &additionalInfo) {
    if (success) {
        Logger::log(Logger::INFO, operation + " succeeded. " + additionalInfo);
    } else {
        Logger::log(Logger::ERROR, operation + " failed. " + additionalInfo);
    }
}

std::string TemplateLoader::encryptTemplate(const std::string &templateContent) {
    std::string key = "your_encryption_key"; 
    return EncryptionUtils::encryptString(templateContent, key);
}

std::string TemplateLoader::decryptTemplate(const std::string &encryptedContent) {
    std::string key = "your_encryption_key"; 
    return EncryptionUtils::decryptString(encryptedContent, key);
}


