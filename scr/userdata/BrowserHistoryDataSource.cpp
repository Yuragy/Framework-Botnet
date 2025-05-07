#include "BrowserHistoryDataSource.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <sqlite3.h>

std::unordered_map<std::string, std::string> BrowserHistoryDataSource::collect(const std::string &userId) {
    Logger::log(Logger::INFO, "Collecting browser history for user: " + userId);
    std::unordered_map<std::string, std::string> browserHistory;

    try {
        auto chromeHistory = collectChromeHistory();
        browserHistory.insert(chromeHistory.begin(), chromeHistory.end());

        auto firefoxHistory = collectFirefoxHistory();
        browserHistory.insert(firefoxHistory.begin(), firefoxHistory.end());
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Failed to collect browser history: " + std::string(e.what()));
    }

    return browserHistory;
}

std::unordered_map<std::string, std::string> BrowserHistoryDataSource::collectChromeHistory() {
    std::string chromeHistoryPath = std::getenv("USERPROFILE");
    chromeHistoryPath += "/AppData/Local/Google/Chrome/User Data/Default/History";

    std::string query = "SELECT url, title FROM urls ORDER BY last_visit_time DESC LIMIT 1000";
    return collectHistoryFromSQLite(chromeHistoryPath, query);
}

std::unordered_map<std::string, std::string> BrowserHistoryDataSource::collectFirefoxHistory() {
    std::unordered_map<std::string, std::string> history;
    std::string firefoxHistoryPath = std::getenv("APPDATA");
    firefoxHistoryPath += "/Mozilla/Firefox/Profiles/";

    for (const auto &entry : std::filesystem::directory_iterator(firefoxHistoryPath)) {
        if (entry.is_directory()) {
            std::string profilePath = entry.path().string() + "/places.sqlite";
            if (std::filesystem::exists(profilePath)) {
                std::string query = "SELECT url, title FROM moz_places ORDER BY last_visit_date DESC LIMIT 1000";
                auto profileHistory = collectHistoryFromSQLite(profilePath, query);
                history.insert(profileHistory.begin(), profileHistory.end());
            }
        }
    }

    return history;
}

std::unordered_map<std::string, std::string> BrowserHistoryDataSource::collectHistoryFromSQLite(const std::string &dbPath, const std::string &query) {
    std::unordered_map<std::string, std::string> history;

    if (!std::filesystem::exists(dbPath)) {
        throw std::runtime_error("Database file not found: " + dbPath);
    }

    sqlite3 *db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + dbPath);
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Failed to prepare query: " + query);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        history[url] = title;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return history;
}


