#include "DatabaseManager.h"
#include "Logger.h"
#include <iostream>
#include <mysql/mysql.h>
#include <future>
#include <vector>
#include <stdexcept>

DatabaseManager::DatabaseManager(const std::string &connectionString) : conn(nullptr), connectionString(connectionString) {}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

void DatabaseManager::connect() {
    try {
        conn = mysql_init(nullptr);
        if (conn == nullptr) {
            logError("mysql_init() failed");
            throw std::runtime_error("mysql_init() failed");
        }

        std::string server, user, password, database;
        parseConnectionString(connectionString, server, user, password, database);

        if (mysql_real_connect(conn, server.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, nullptr, 0) == nullptr) {
            logError("mysql_real_connect() failed: " + std::string(mysql_error(conn)));
            mysql_close(conn);
            throw std::runtime_error("mysql_real_connect() failed");
        }
        Logger::log(Logger::INFO, "Successfully connected to database");
    } catch (const std::exception &e) {
        logError("Exception in connect: " + std::string(e.what()));
        throw; 
    }
}

void DatabaseManager::disconnect() {
    try {
        if (conn != nullptr) {
            mysql_close(conn);
            conn = nullptr;
            Logger::log(Logger::INFO, "Disconnected from database");
        }
    } catch (const std::exception &e) {
        logError("Exception in disconnect: " + std::string(e.what()));
    }
}

std::future<void> DatabaseManager::executeQueryAsync(const std::string &query) {
    return std::async(std::launch::async, &DatabaseManager::executeQuery, this, query);
}

std::future<std::vector<std::vector<std::string>>> DatabaseManager::executeSelectQueryAsync(const std::string &query) {
    return std::async(std::launch::async, &DatabaseManager::executeSelectQuery, this, query);
}

void DatabaseManager::beginTransaction() {
    try {
        if (conn == nullptr) {
            logError("Not connected to database");
            throw std::runtime_error("Not connected to database");
        }

        if (mysql_query(conn, "START TRANSACTION")) {
            logError("Failed to start transaction: " + std::string(mysql_error(conn)));
            throw std::runtime_error("Failed to start transaction");
        }

        Logger::log(Logger::INFO, "Transaction started");
    } catch (const std::exception &e) {
        logError("Exception in beginTransaction: " + std::string(e.what()));
        throw;
    }
}

void DatabaseManager::commitTransaction() {
    try {
        if (conn == nullptr) {
            logError("Not connected to database");
            throw std::runtime_error("Not connected to database");
        }

        if (mysql_query(conn, "COMMIT")) {
            logError("Failed to commit transaction: " + std::string(mysql_error(conn)));
            throw std::runtime_error("Failed to commit transaction");
        }

        Logger::log(Logger::INFO, "Transaction committed");
    } catch (const std::exception &e) {
        logError("Exception in commitTransaction: " + std::string(e.what()));
        throw;
    }
}

void DatabaseManager::rollbackTransaction() {
    try {
        if (conn == nullptr) {
            logError("Not connected to database");
            throw std::runtime_error("Not connected to database");
        }

        if (mysql_query(conn, "ROLLBACK")) {
            logError("Failed to rollback transaction: " + std::string(mysql_error(conn)));
            throw std::runtime_error("Failed to rollback transaction");
        }

        Logger::log(Logger::INFO, "Transaction rolled back");
    } catch (const std::exception &e) {
        logError("Exception in rollbackTransaction: " + std::string(e.what()));
        throw;
    }
}

void DatabaseManager::executeQuery(const std::string &query) {
    try {
        if (conn == nullptr) {
            logError("Not connected to database");
            throw std::runtime_error("Not connected to database");
        }

        if (mysql_query(conn, query.c_str())) {
            logError("Query failed: " + std::string(mysql_error(conn)));
            throw std::runtime_error("Query failed");
        }

        Logger::log(Logger::INFO, "Query executed successfully: " + query);
    } catch (const std::exception &e) {
        logError("Exception in executeQuery: " + std::string(e.what()));
        throw;
    }
}

std::vector<std::vector<std::string>> DatabaseManager::executeSelectQuery(const std::string &query) {
    try {
        if (conn == nullptr) {
            logError("Not connected to database");
            throw std::runtime_error("Not connected to database");
        }

        if (mysql_query(conn, query.c_str())) {
            logError("Query failed: " + std::string(mysql_error(conn)));
            throw std::runtime_error("Query failed");
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result == nullptr) {
            logError("mysql_store_result() failed: " + std::string(mysql_error(conn)));
            throw std::runtime_error("mysql_store_result() failed");
        }

        int num_fields = mysql_num_fields(result);
        MYSQL_ROW row;
        std::vector<std::vector<std::string>> table;

        while ((row = mysql_fetch_row(result))) {
            std::vector<std::string> rowData;
            for (int i = 0; i < num_fields; i++) {
                rowData.push_back(row[i] ? row[i] : "NULL");
            }
            table.push_back(rowData);
        }

        mysql_free_result(result);
        Logger::log(Logger::INFO, "Select query executed successfully: " + query);

        return table;
    } catch (const std::exception &e) {
        logError("Exception in executeSelectQuery: " + std::string(e.what()));
        throw;  
    }
}

void DatabaseManager::parseConnectionString(const std::string &connectionString, std::string &server, std::string &user, std::string &password, std::string &database) {
    size_t start = 0, end = connectionString.find(';');
    while (end != std::string::npos) {
        std::string token = connectionString.substr(start, end - start);
        size_t delimiter = token.find('=');
        if (delimiter != std::string::npos) {
            std::string key = token.substr(0, delimiter);
            std::string value = token.substr(delimiter + 1);
            if (key == "server") server = value;
            else if (key == "user") user = value;
            else if (key == "password") password = value;
            else if (key == "database") database = value;
        }
        start = end + 1;
        end = connectionString.find(';', start);
    }
}

void DatabaseManager::logError(const std::string &message) {
    Logger::log(Logger::ERROR, message);
}


