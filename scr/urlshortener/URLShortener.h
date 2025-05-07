#ifndef URLSHORTENER_H
#define URLSHORTENER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <list>
#include <chrono>

class URLShortener {
public:
    static std::string shorten(const std::string &url);
    static void setStrategy(const std::string &strategy);
    static void setCacheSize(size_t size);
    static void setAPIKeys(const std::string &bitlyKey, const std::string &tinyurlKey, const std::string &customKey);
    static void setCacheTTL(size_t ttl);

private:
    struct CacheEntry {
        std::string shortenedURL;
        std::chrono::steady_clock::time_point expiry;
    };

    static std::unordered_map<std::string, CacheEntry> cache;
    static std::list<std::string> lruList;
    static std::string strategy;
    static std::mutex cacheMutex;
    static size_t maxCacheSize;
    static size_t cacheTTL;
    static std::string bitlyAPIKey;
    static std::string tinyurlAPIKey;
    static std::string customAPIKey;

    static std::string internalShorten(const std::string &url);
    static std::string externalShorten(const std::string &url);
    static std::string bitlyShorten(const std::string &url);
    static std::string tinyurlShorten(const std::string &url);
    static std::string customShorten(const std::string &url);
    static void cacheURL(const std::string &url, const std::string &shortenedURL);
    static std::string getCachedURL(const std::string &url);
    static void updateLRU(const std::string &url);
    static bool isCacheEntryExpired(const CacheEntry &entry);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif // URLSHORTENER_H


