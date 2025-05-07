#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <list>
#include <functional>

class Cache {
public:
    static std::string get(const std::string &key);
    static void set(const std::string &key, const std::string &value, std::chrono::seconds ttl = std::chrono::seconds(0));
    static void remove(const std::string &key);
    static void clear();
    static void setEvictionStrategy(const std::function<void()> &strategy);
    static void runEviction();
    static void setMaxSize(size_t maxSize);
    static size_t getCurrentSize();
    static size_t getMaxSize();
    static bool exists(const std::string &key);
    static void updateTTL(const std::string &key, std::chrono::seconds ttl);

private:
    struct CacheEntry {
        std::string value;
        std::chrono::system_clock::time_point expiry;
    };

    static std::unordered_map<std::string, CacheEntry> cache;
    static std::list<std::string> lruList;
    static std::mutex cacheMutex;
    static std::function<void()> evictionStrategy;
    static size_t maxSize;

    static bool isExpired(const CacheEntry &entry);
    static void defaultEvictionStrategy();
    static void lruEvictionStrategy();
    static void moveToFront(const std::string &key);
};

#endif // CACHE_H

