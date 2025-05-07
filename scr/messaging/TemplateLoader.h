#ifndef TEMPLATELOADER_H
#define TEMPLATELOADER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <future>
#include <mutex>
#include <chrono>

class TemplateLoader {
public:
    TemplateLoader();

    std::string loadTemplateFromFile(const std::string &filePath, const std::string &format = "txt");
    std::string loadTemplateFromDatabase(const std::string &templateName);
    std::future<std::string> loadTemplateFromFileAsync(const std::string &filePath, const std::string &format = "txt");
    std::future<std::string> loadTemplateFromDatabaseAsync(const std::string &templateName);
    std::string fillTemplate(const std::string &templateStr, const std::unordered_map<std::string, std::string> &data);
    
    void cacheTemplate(const std::string &templateName, const std::string &templateContent, std::chrono::seconds ttl);
    
    std::string getTemplateFromCache(const std::string &templateName);
    std::string loadLocalizedTemplate(const std::string &templateName, const std::string &locale);
    std::string loadTemplateFromAPI(const std::string &apiUrl);

private:

    std::string replacePlaceholders(const std::string &templateStr, const std::unordered_map<std::string, std::string> &data);

    void logOperation(const std::string &operation, bool success, const std::string &additionalInfo = "");

    std::string encryptTemplate(const std::string &templateContent);
    std::string decryptTemplate(const std::string &encryptedContent);

    struct CachedTemplate {
        std::string content;
        std::chrono::system_clock::time_point expiryTime;
    };

    std::unordered_map<std::string, CachedTemplate> templateCache;
    std::mutex cacheMutex;
};

#endif // TEMPLATELOADER_H


