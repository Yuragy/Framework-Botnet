#include "URLShortener.h"
#include "Logger.h"
#include <curl/curl.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <ctime>

std::unordered_map<std::string, URLShortener::CacheEntry> URLShortener::cache;
std::list<std::string> URLShortener::lruList;
std::string URLShortener::strategy = "internal";
std::mutex URLShortener::cacheMutex;
size_t URLShortener::maxCacheSize = 100;
size_t URLShortener::cacheTTL = 3600;
std::string URLShortener::bitlyAPIKey = "";
std::string URLShortener::tinyurlAPIKey = "";
std::string URLShortener::customAPIKey = "";

std::string URLShortener::shorten(const std::string &url) {
    Logger::log(Logger::INFO, "Shortening URL: " + url);

    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        std::string cachedURL = getCachedURL(url);
        if (!cachedURL.empty()) {
            Logger::log(Logger::INFO, "URL found in cache: " + cachedURL);
            return cachedURL;
        }
    }

    std::string shortenedURL;
    try {
        if (strategy == "internal") {
            shortenedURL = internalShorten(url);
        } else if (strategy == "bitly") {
            shortenedURL = bitlyShorten(url);
        } else if (strategy == "tinyurl") {
            shortenedURL = tinyurlShorten(url);
        } else if (strategy == "custom") {
            shortenedURL = customShorten(url);
        } else {
            shortenedURL = externalShorten(url);
        }
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            cacheURL(url, shortenedURL);
        }
        Logger::log(Logger::INFO, "Shortened URL: " + shortenedURL);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Error shortening URL: " + std::string(e.what()));
        throw;
    }
    
    return shortenedURL;
}

void URLShortener::setStrategy(const std::string &newStrategy) {
    strategy = newStrategy;
    Logger::log(Logger::INFO, "URL shortening strategy set to: " + strategy);
}

void URLShortener::setCacheSize(size_t size) {
    maxCacheSize = size;
    Logger::log(Logger::INFO, "URL cache size set to: " + std::to_string(maxCacheSize));
}

void URLShortener::setAPIKeys(const std::string &bitlyKey, const std::string &tinyurlKey, const std::string &customKey) {
    bitlyAPIKey = bitlyKey;
    tinyurlAPIKey = tinyurlKey;
    customAPIKey = customKey;
    Logger::log(Logger::INFO, "API keys set for Bitly, TinyURL, and Custom");
}

void URLShortener::setCacheTTL(size_t ttl) {
    cacheTTL = ttl;
    Logger::log(Logger::INFO, "Cache TTL set to: " + std::to_string(cacheTTL) + " seconds");
}

std::string URLShortener::internalShorten(const std::string &url) {
    std::hash<std::string> hasher;
    return "http://short.url/" + std::to_string(hasher(url));
}

std::string URLShortener::externalShorten(const std::string &url) {
    throw std::runtime_error("External URL shortening not implemented");
}

size_t URLShortener::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string URLShortener::bitlyShorten(const std::string &url) {
    if (bitlyAPIKey.empty()) {
        throw std::runtime_error("Bitly API key is not set");
    }

    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string bitlyURL = "https://api-ssl.bitly.com/v4/shorten";
    std::string json = "{\"long_url\": \"" + url + "\"}";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + bitlyAPIKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, bitlyURL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        size_t pos = readBuffer.find("\"link\":");
        if (pos != std::string::npos) {
            size_t start = readBuffer.find("\"", pos + 7) + 1;
            size_t end = readBuffer.find("\"", start);
            return readBuffer.substr(start, end - start);
        } else {
            throw std::runtime_error("Failed to parse Bitly response");
        }
    } else {
        throw std::runtime_error("Failed to initialize curl");
    }
}

std::string URLShortener::tinyurlShorten(const std::string &url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string tinyurlRequest = "http://tinyurl.com/api-create.php?url=" + url;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, tinyurlRequest.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return readBuffer;
    } else {
        throw std**.runtime_error("Failed to initialize curl");
    }
}

std::string URLShortener::customShorten(const std::string &url) {
    if (customAPIKey.empty()) {
        throw std::runtime_error("Custom API key is not set");
    }

    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string customURL = "https://api.customshortener.com/shorten";
    std::string json = "{\"long_url\": \"" + url + "\"}";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + customAPIKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, customURL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        size_t pos = readBuffer.find("\"shortened_url\":");
        if (pos != std::string::npos) {
            size_t start = readBuffer.find("\"", pos + 16) + 1;
            size_t end = readBuffer.find("\"", start);
            return readBuffer.substr(start, end - start);
        } else {
            throw std::runtime_error("Failed to parse custom shortener response");
        }
    } else {
        throw std::runtime_error("Failed to initialize curl");
    }
}

void URLShortener::cacheURL(const std::string &url, const std::string &shortenedURL) {
    auto now = std::chrono::steady_clock::now();
    CacheEntry entry = {shortenedURL, now + std::chrono::seconds(cacheTTL)};
    
    if (cache.size() >= maxCacheSize) {
        std::string oldest = lruList.back();
        lruList.pop_back();
        cache.erase(oldest);
    }
    
    cache[url] = entry;
    lruList.push_front(url);
    Logger::log(Logger::INFO, "URL cached: " + url + " -> " + shortenedURL);
}

std::string URLShortener::getCachedURL(const std::string &url) {
    auto it = cache.find(url);
    if (it != cache.end() && !isCacheEntryExpired(it->second)) {
        updateLRU(url);
        return it->second.shortenedURL;
    }
    return "";
}

void URLShortener::updateLRU(const std::string &url) {
    lruList.remove(url);
    lruList.push_front(url);
}

bool URLShortener::isCacheEntryExpired(const CacheEntry &entry) {
    return std::chrono::steady_clock::now() > entry.expiry;
}


