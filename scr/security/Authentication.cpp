#include "Authentication.h"
#include "Logger.h"
#include <iostream>
#include <stdexcept>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include <map>
#include <random>

std::map<std::string, std::pair<std::string, std::string>> userDatabase = {
    {"user1", {"5e884898da28047151d0e56f8dc6292773603d0d6aabbddc250eb8a56b3a024b", "somesalt"}},
    {"user2", {"d8578edf8458ce06fbc5bb76a58c5ca4a6990bb8e5d6f68f1d4a476d7f84a933", "othersalt"}}
};

std::map<std::string, std::string> passwordLastChanged = {
    {"user1", "2023-01-01"},
    {"user2", "2023-02-01"}
};

bool Authentication::authenticate(const std::string &username, const std::string &password) {
    Logger::log(Logger::INFO, "Authenticating user: " + username);

    if (!checkPasswordComplexity(password)) {
        Logger::log(Logger::WARNING, "Password complexity check failed for user: " + username);
        return false;
    }

    if (isPasswordExpired(username)) {
        Logger::log(Logger::WARNING, "Password expired for user: " + username);
        return false;
    }

    auto storedCredentials = getStoredPasswordHash(username);
    if (storedCredentials.empty()) {
        Logger::log(Logger::ERROR, "User not found: " + username);
        return false;
    }

    std::string hashedPassword = hashPasswordWithSalt(password, storedCredentials.second);

    return verifyCredentials(username, hashedPassword);
}

bool Authentication::checkPasswordComplexity(const std::string &password) {
    if (password.length() < 8) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        if (isupper(ch)) hasUpper = true;
        if (islower(ch)) hasLower = true;
        if (isdigit(ch)) hasDigit = true;
        if (ispunct(ch)) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

std::string Authentication::hashPassword(const std::string &password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string Authentication::hashPasswordWithSalt(const std::string &password, const std::string &salt) {
    std::string saltedPassword = password + salt;
    return hashPassword(saltedPassword);
}

std::string Authentication::generateSalt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(33, 126); 

    std::string salt;
    for (int i = 0; i < 16; ++i) {
        salt += static_cast<char>(dis(gen));
    }
    return salt;
}

bool Authentication::verifyCredentials(const std::string &username, const std::string &hashedPassword) {
    auto storedCredentials = getStoredPasswordHash(username);
    if (storedCredentials.empty()) {
        Logger::log(Logger::ERROR, "User not found: " + username);
        return false;
    }

    if (storedCredentials.first == hashedPassword) {
        Logger::log(Logger::INFO, "User authenticated successfully: " + username);
        return true;
    } else {
        Logger::log(Logger::ERROR, "Password mismatch for user: " + username);
        return false;
    }
}

std::pair<std::string, std::string> Authentication::getStoredPasswordHash(const std::string &username) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) {
        return it->second;
    }
    return {};
}

void Authentication::storePasswordHash(const std::string &username, const std::string &hashedPassword) {
    std::string salt = generateSalt();
    userDatabase[username] = {hashPasswordWithSalt(hashedPassword, salt), salt};
}

bool Authentication::isPasswordExpired(const std::string &username) {
    auto it = passwordLastChanged.find(username);
    if (it == passwordLastChanged.end()) {
        Logger::log(Logger::ERROR, "No password change date found for user: " + username);
        return true; 
    }

    std::tm lastChanged = {};
    std::istringstream ss(it->second);
    ss >> std::get_time(&lastChanged, "%Y-%m-%d");

    std::time_t now = std::time(nullptr);
    std::tm nowTm = *std::localtime(&now);

    std::chrono::system_clock::time_point lastChangedTime = std::chrono::system_clock::from_time_t(std::mktime(&lastChanged));
    std::chrono::system_clock::time_point nowTime = std::chrono::system_clock::from_time_t(std::mktime(&nowTm));

    std::chrono::duration<double> diff = nowTime - lastChangedTime;
    double daysDiff = diff.count() / (60 * 60 * 24);

    if (daysDiff > 90) { 
        Logger::log(Logger::WARNING, "Password for user " + username + " expired " + std::to_string(daysDiff) + " days ago");
        return true;
    }

    return false;
}

bool Authentication::updatePassword(const std::string &username, const std::string &newPassword) {
    if (!checkPasswordComplexity(newPassword)) {
        Logger::log(Logger::WARNING, "Password complexity check failed for user: " + username);
        return false;
    }

    std::string salt = generateSalt();
    std::string hashedPassword = hashPasswordWithSalt(newPassword, salt);
    storePasswordHash(username, hashedPassword);
    setPasswordLastChangedDate(username);
    Logger::log(Logger::INFO, "Password updated successfully for user: " + username);

    return true;
}

std::string Authentication::getPasswordLastChangedDate(const std::string &username) {
    auto it = passwordLastChanged.find(username);
    if (it != passwordLastChanged.end()) {
        return it->second;
    }
    return "";
}

void Authentication::setPasswordLastChangedDate(const std::string &username) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d");
    passwordLastChanged[username] = ss.str();
}

void Authentication::logOperation(const std::string &operation, bool success, const std::string &additionalInfo) {
    if (success) {
        Logger::log(Logger::INFO, operation + " succeeded. " + additionalInfo);
    } else {
        Logger::log(Logger::ERROR, operation + " failed. " + additionalInfo);
    }
}


