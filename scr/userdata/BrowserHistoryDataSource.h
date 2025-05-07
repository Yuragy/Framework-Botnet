#ifndef BROWSERHISTORYDATASOURCE_H
#define BROWSERHISTORYDATASOURCE_H

#include "UserDataCollector.h"
#include <unordered_map>
#include <string>

class BrowserHistoryDataSource : public DataSource {
public:
    std::unordered_map<std::string, std::string> collect(const std::string &userId) override;

private:
    std::unordered_map<std::string, std::string> collectChromeHistory();
    std::unordered_map<std::string, std::string> collectFirefoxHistory();

    std::unordered_map<std::string, std::string> collectHistoryFromSQLite(const std::string &dbPath, const std::string &query);
};

#endif // BROWSERHISTORYDATASOURCE_H


