#ifndef ENCRYPTIONUTILS_H
#define ENCRYPTIONUTILS_H

#include <string>
#include <vector>

class EncryptionUtils {
public:
    static void encryptFile(const std::string &filePath, const std::string &key, const std::string &outputFilePath);
    static void decryptFile(const std::string &filePath, const std::string &key, const std::string &outputFilePath);
    static std::vector<unsigned char> generateKey(size_t length);
    static std::string obfuscate(const std::string &input);
    static std::string deobfuscate(const std::string &input);
    static void polymorphicEncryptDecrypt(const std::string &input, const std::string &key, bool isEncrypt);
    static std::string encryptString(const std::string &input, const std::string &key);
    static std::string decryptString(const std::string &input, const std::string &key);
    static std::string generateSignature(const std::string &data, const std::string &privateKey);
    static bool verifySignature(const std::string &data, const std::string &signature, const std::string &publicKey);
    static std::vector<unsigned char> deriveKey(const std::string &password, const std::vector<unsigned char> &salt, int iterations, size_t keyLength);
    static void secureDelete(const std::string &filePath);

private:
    static bool generateIv(unsigned char* iv, int size);
    static void handleErrors();
    static void encryptDecrypt(std::ifstream &inFile, std::ofstream &outFile, const std::string &key, bool isEncrypt);
    class Encryptor {
    public:
        virtual void process(std::ifstream &inFile, std::ofstream &outFile, const std::string &key) = 0;
    };

    class AesEncryptor : public Encryptor {
    public:
        void process(std::ifstream &inFile, std::ofstream &outFile, const std::string &key) override;
    };

    class AesDecryptor : public Encryptor {
    public:
        void process(std::ifstream &inFile, std::ofstream &outFile, const std::string &key) override;
    };
};

#endif // ENCRYPTIONUTILS_H



