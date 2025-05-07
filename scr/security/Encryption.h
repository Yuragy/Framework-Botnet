#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <vector>

class Encryption {
public:
    static std::string encrypt(const std::string &data, const std::string &key, const std::string &algorithm = "aes-256-cbc");
    static std::string decrypt(const std::string &data, const std::string &key, const std::string &algorithm = "aes-256-cbc");
    
private:
    static void generateKeyAndIV(const std::string &key, unsigned char *outKey, unsigned char *outIV, const std::string &algorithm);
    static void handleOpenSSLErrors();
    static std::vector<unsigned char> stringToBytes(const std::string &str);
    static std::string bytesToString(const std::vector<unsigned char> &bytes);
    static void getKeyAndIVSizes(const std::string &algorithm, int &keySize, int &ivSize);
};

#endif // ENCRYPTION_H


