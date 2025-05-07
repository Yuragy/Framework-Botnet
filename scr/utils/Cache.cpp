#include "Cache.h"
#include "Logger.h"
#include <stdexcept>

std::unordered_map<std::string, Cache::CacheEntry> Cache::cache;
std::list<std::string> Cache::lruList;
std::mutex Cache::cacheMutex;
std::function<void()> Cache::evictionStrategy = defaultEvictionStrategy;
size_t Cache::maxSize = 100;

std::string Cache::get(const std::string &key) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cache.find(key);
    if (it != cache.end()) {
        if (!isExpired(it->second)) {
            moveToFront(key);
            Logger::log(Logger::INFO, "Cache hit for key: " + key);
            return it->second.value;
        } else {
            cache.erase(it);
            Logger::log(Logger::INFO, "Cache key expired: " + key);
            throw std::runtime_error("Cache key expired");
        }
    }
    Logger::log(Logger::WARNING, "Cache key not found: " + key);
    throw std::runtime_error("Cache key not found");
}

void Cache::set(const std::string &key, const std::string &value, std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    if (cache.size() >= maxSize) {
        evictionStrategy();
    }
    auto expiry = ttl.count() > 0 ? std::chrono::system_clock::now() + ttl : std::chrono::system_clock::time_point::max();
    cache[key] = {value, expiry};
    lruList.push_front(key);
    Logger::log(Logger::INFO, "Cache set: " + key + " = " + value + " with TTL: " + std::to_string(ttl.count()) + " seconds");
}

void Cache::remove(const std::string &key) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.erase(key);
    lruList.remove(key);
    Logger::log(Logger::INFO, "Cache key removed: " + key);
}

void Cache::clear() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
    lruList.clear();
    Logger::log(Logger::INFO, "Cache cleared");
}

void Cache::setEvictionStrategy(const std::function<void()> &strategy) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    evictionStrategy = strategy;
    Logger::log(Logger::INFO, "Cache eviction strategy set");
}

void Cache::runEviction() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    evictionStrategy();
}

void Cache::setMaxSize(size_t newSize) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    maxSize = newSize;
    Logger::log(Logger::INFO, "Cache max size set to: " + std::to_string(maxSize));
}

size_t Cache::getCurrentSize() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return cache.size();
}

size_t Cache::getMaxSize() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return maxSize;
}

bool Cache::exists(const std::string &key) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cache.find(key);
    if (it != cache.end() && !isExpired(it->second)) {
        moveToFront(key);
        return true;
    }
    return false;
}

void Cache::updateTTL(const std::string &key, std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cache.find(key);
    if (it != cache.end()) {
        it->second.expiry = ttl.count() > 0 ? std::chrono::system_clock::now() + ttl : std::chrono::system_clock::time_point::max();
        Logger::log(Logger::INFO, "Cache TTL updated for key: " + key + " to " + std::to_string(ttl.count()) + " seconds");
    } else {
        Logger::log(Logger::WARNING, "Cache key not found for TTL update: " + key);
        throw std::runtime_error("Cache key not found for TTL update: " + key);
    }
}

bool Cache::isExpired(const CacheEntry &entry) {
    return std::chrono::system_clock::now() > entry.expiry;
}

void Cache::defaultEvictionStrategy() {
    auto now = std::chrono::system_clock::now();
    for (auto it = cache.begin(); it != cache.end(); ) {
        if (isExpired(it->second)) {
            Logger::log(Logger::INFO, "Evicting expired cache key: " + it->first);
            lruList.remove(it->first);
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
    if (cache.size() > maxSize) {
        lruEvictionStrategy();
    }
}

void Cache::lruEvictionStrategy() {
    while (cache.size() > maxSize) {
        std::string key = lruList.back();
        lruList.pop_back();
        cache.erase(key);
        Logger::log(Logger::INFO, "Evicting LRU cache key: " + key);
    }
}

void Cache::moveToFront(const std::string &key) {
    lruList.remove(key);
    lruList.push_front(key);
}

