#include "Config.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <tinyxml2.h>

using namespace tinyxml2;

std::unordered_map<std::string, std::string> Config::config;
std::mutex Config::configMutex;

std::string Config::get(const std::string &key) {
    std::lock_guard<std::mutex> lock(configMutex);
    auto it = config.find(key);
    if (it != config.end()) {
        return it->second;
    }
    Logger::log(Logger::WARNING, "Config key not found: " + key);
    throw std::runtime_error("Config key not found: " + key);
}

void Config::set(const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> lock(configMutex);
    config[key] = value;
    Logger::log(Logger::INFO, "Config set: " + key + " = " + value);
}

void Config::loadFromFile(const std::string &filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        handleFileError(filePath, "open");
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            set(key, value);
        }
    }
    logConfig("loaded from", filePath);
}

void Config::saveToFile(const std::string &filePath) {
    std::lock_guard<std::mutex> lock(configMutex);
    std::ofstream outFile(filePath);
    if (!outFile) {
        handleFileError(filePath, "write");
    }

    for (const auto &pair : config) {
        outFile << pair.first << "=" << pair.second << "\n";
    }
    logConfig("saved to", filePath);
}

void Config::loadFromJsonFile(const std::string &filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        handleFileError(filePath, "open");
    }

    nlohmann::json jsonConfig;
    inFile >> jsonConfig;

    for (auto& [key, value] : jsonConfig.items()) {
        set(key, value.get<std::string>());
    }
    logConfig("loaded from JSON file", filePath);
}

void Config::saveToJsonFile(const std::string &filePath) {
    std::lock_guard<std::mutex> lock(configMutex);
    std::ofstream outFile(filePath);
    if (!outFile) {
        handleFileError(filePath, "write");
    }

    nlohmann::json jsonConfig;
    for (const auto &pair : config) {
        jsonConfig[pair.first] = pair.second;
    }
    outFile << jsonConfig.dump(4);
    logConfig("saved to JSON file", filePath);
}

void Config::loadFromYamlFile(const std::string &filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        handleFileError(filePath, "open");
    }

    YAML::Node yamlConfig = YAML::LoadFile(filePath);
    for (YAML::const_iterator it = yamlConfig.begin(); it != yamlConfig.end(); ++it) {
        set(it->first.as<std::string>(), it->second.as<std::string>());
    }
    logConfig("loaded from YAML file", filePath);
}

void Config::saveToYamlFile(const std::string &filePath) {
    std::lock_guard<std::mutex> lock(configMutex);
    std::ofstream outFile(filePath);
    if (!outFile) {
        handleFileError(filePath, "write");
    }

    YAML::Node yamlConfig;
    for (const auto &pair : config) {
        yamlConfig[pair.first] = pair.second;
    }
    outFile << yamlConfig;
    logConfig("saved to YAML file", filePath);
}

void Config::loadFromXmlFile(const std::string &filePath) {
    XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filePath.c_str());
    if (eResult != XML_SUCCESS) {
        handleFileError(filePath, "open");
    }

    XMLNode *pRoot = xmlDoc.FirstChild();
    if (pRoot == nullptr) {
        Logger::log(Logger::ERROR, "No root element in XML config file: " + filePath);
        throw std::runtime_error("No root element in XML config file: " + filePath);
    }

    XMLElement *pElement = pRoot->FirstChildElement("Config");
    while (pElement != nullptr) {
        const char *key = pElement->Attribute("key");
        const char *value = pElement->Attribute("value");
        if (key && value) {
            set(key, value);
        }
        pElement = pElement->NextSiblingElement("Config");
    }
    logConfig("loaded from XML file", filePath);
}

void Config::saveToXmlFile(const std::string &filePath) {
    std::lock_guard<std::mutex> lock(configMutex);
    XMLDocument xmlDoc;
    XMLNode *pRoot = xmlDoc.NewElement("Configurations");
    xmlDoc.InsertFirstChild(pRoot);

    for (const auto &pair : config) {
        XMLElement *pElement = xmlDoc.NewElement("Config");
        pElement->SetAttribute("key", pair.first.c_str());
        pElement->SetAttribute("value", pair.second.c_str());
        pRoot->InsertEndChild(pElement);
    }

    XMLError eResult = xmlDoc.SaveFile(filePath.c_str());
    if (eResult != XML_SUCCESS) {
        handleFileError(filePath, "write");
    }
    logConfig("saved to XML file", filePath);
}

void Config::logConfig(const std::string &action, const std::string &filePath) {
    Logger::log(Logger::INFO, "Config " + action + ": " + filePath);
}

void Config::handleFileError(const std::string &filePath, const std::string &operation) {
    Logger::log(Logger::ERROR, "Failed to " + operation + " config file: " + filePath);
    throw std::runtime_error("Failed to " + operation + " config file: " + filePath);
}

