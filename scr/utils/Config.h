#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
#include <mutex>

class Config {
public:
    static std::string get(const std::string &key);
    static void set(const std::string &key, const std::string &value);
    static void loadFromFile(const std::string &filePath);
    static void saveToFile(const std::string &filePath);
    static void loadFromJsonFile(const std::string &filePath);
    static void saveToJsonFile(const std::string &filePath);
    static void loadFromYamlFile(const std::string &filePath);
    static void saveToYamlFile(const std::string &filePath);
    static void loadFromXmlFile(const std::string &filePath);
    static void saveToXmlFile(const std::string &filePath);

private:
    static std::unordered_map<std::string, std::string> config;
    static std::mutex configMutex;

    static void logConfig(const std::string &action, const std::string &filePath);
    static void handleFileError(const std::string &filePath, const std::string &operation);
};

#endif // CONFIG_H

