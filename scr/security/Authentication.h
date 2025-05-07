#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <string>

class Authentication {
public:

    static bool authenticate(const std::string &username, const std::string &password);
    static bool checkPasswordComplexity(const std::string &password);
    static std::string hashPassword(const std::string &password);
    static bool isPasswordExpired(const std::string &username);
    static bool updatePassword(const std::string &username, const std::string &newPassword);

private:
    static bool verifyCredentials(const std::string &username, const std::string &hashedPassword);
    static std::string getStoredPasswordHash(const std::string &username);
    static void storePasswordHash(const std::string &username, const std::string &hashedPassword);
    static void logOperation(const std::string &operation, bool success, const std::string &additionalInfo = "");
    static std::string getPasswordLastChangedDate(const std::string &username);
    static void setPasswordLastChangedDate(const std::string &username);
    static std::string generateSalt();
    static std::string hashPasswordWithSalt(const std::string &password, const std::string &salt);
};

#endif // AUTHENTICATION_H



