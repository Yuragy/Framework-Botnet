#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <future>
#include <memory>
#include <mysql/mysql.h>

class DatabaseManager {
public:
    DatabaseManager(const std::string &connectionString);
    ~DatabaseManager();

    void connect();
    void disconnect();
    std::future<void> executeQueryAsync(const std::string &query);
    std::future<std::vector<std::vector<std::string>>> executeSelectQueryAsync(const std::string &query);
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

private:
    MYSQL *conn;
    std::string connectionString;

    void parseConnectionString(const std::string &connectionString, std::string &server, std::string &user, std::string &password, std::string &database);
    void logError(const std::string &message);
    void executeQuery(const std::string &query);
    std::vector<std::vector<std::string>> executeSelectQuery(const std::string &query);
};

#endif // DATABASEMANAGER_H


