#include "EncryptionUtils.h"
#include "Logger.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <filesystem>

void EncryptionUtils::encryptFile(const std::string &filePath, const std::string &key, const std::string &outputFilePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        Logger::log(Logger::ERROR, "Failed to open input file for encryption: " + filePath);
        throw std::runtime_error("Failed to open input file for encryption: " + filePath);
    }

    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        Logger::log(Logger::ERROR, "Failed to open output file for encryption: " + outputFilePath);
        throw std::runtime_error("Failed to open output file for encryption: " + outputFilePath);
    }

    AesEncryptor encryptor;
    encryptor.process(inFile, outFile, key);
}

void EncryptionUtils::decryptFile(const std::string &filePath, const std::string &key, const std::string &outputFilePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        Logger::log(Logger::ERROR, "Failed to open input file for decryption: " + filePath);
        throw std::runtime_error("Failed to open input file for decryption: " + filePath);
    }

    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        Logger::log(Logger::ERROR, "Failed to open output file for decryption: " + outputFilePath);
        throw std::runtime_error("Failed to open output file for decryption: " + outputFilePath);
    }

    AesDecryptor decryptor;
    decryptor.process(inFile, outFile, key);
}

std::vector<unsigned char> EncryptionUtils::generateKey(size_t length) {
    std::vector<unsigned char> key(length);
    if (!RAND_bytes(key.data(), length)) {
        Logger::log(Logger::ERROR, "Failed to generate key");
        throw std::runtime_error("Failed to generate key");
    }
    return key;
}

bool EncryptionUtils::generateIv(unsigned char* iv, int size) {
    if (!RAND_bytes(iv, size)) {
        Logger::log(Logger::ERROR, "Failed to generate IV");
        return false;
    }
    return true;
}

void EncryptionUtils::handleErrors() {
    unsigned long errCode;
    while ((errCode = ERR_get_error())) {
        char *err = ERR_error_string(errCode, NULL);
        Logger::log(Logger::ERROR, "OpenSSL Error: " + std::string(err));
    }
    throw std::runtime_error("OpenSSL Error");
}

void EncryptionUtils::encryptDecrypt(std::ifstream &inFile, std::ofstream &outFile, const std::string &key, bool isEncrypt) {
    const int keyLength = 32; 
    const int ivLength = 16;
    unsigned char iv[ivLength];

    if (isEncrypt) {
        if (!generateIv(iv, ivLength)) {
            throw std::runtime_error("Failed to generate IV for encryption");
        }
        outFile.write(reinterpret_cast<char*>(iv), ivLength);
    } else {
        inFile.read(reinterpret_cast<char*>(iv), ivLength);
    }

    std::vector<unsigned char> buffer(1024);
    std::vector<unsigned char> outBuffer(1024 + EVP_MAX_BLOCK_LENGTH);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (1 != EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, NULL, NULL, isEncrypt)) handleErrors();

    unsigned char keyBytes[keyLength];
    memset(keyBytes, 0, keyLength);
    memcpy(keyBytes, key.data(), std::min(key.size(), size_t(keyLength)));

    if (1 != EVP_CipherInit_ex(ctx, NULL, NULL, keyBytes, iv, isEncrypt)) handleErrors();

    int outLen;

    while (inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size())) {
        if (1 != EVP_CipherUpdate(ctx, outBuffer.data(), &outLen, buffer.data(), inFile.gcount())) handleErrors();
        outFile.write(reinterpret_cast<char*>(outBuffer.data()), outLen);
    }

    if (1 != EVP_CipherFinal_ex(ctx, outBuffer.data(), &outLen)) handleErrors();
    outFile.write(reinterpret_cast<char*>(outBuffer.data()), outLen);

    EVP_CIPHER_CTX_free(ctx);
}

std::string EncryptionUtils::obfuscate(const std::string &input) {
    std::string obfuscated = input;
    for (char &c : obfuscated) {
        c ^= 0xAA;
    }
    return obfuscated;
}

std::string EncryptionUtils::deobfuscate(const std::string &input) {
    return obfuscate(input);
}

void EncryptionUtils::polymorphicEncryptDecrypt(const std::string &input, const std::string &key, bool isEncrypt) {
    std::ifstream inFile(input, std::ios::binary);
    std::string output = input + (isEncrypt ? ".enc" : ".dec");
    std::ofstream outFile(output, std::ios::binary);

    if (!inFile || !outFile) {
        Logger::log(Logger::ERROR, "Failed to open files for polymorphic encrypt/decrypt");
        throw std::runtime_error("Failed to open files for polymorphic encrypt/decrypt");
    }

    std::unique_ptr<Encryptor> processor;
    if (isEncrypt) {
        processor = std::make_unique<AesEncryptor>();
    } else {
        processor = std::make_unique<AesDecryptor>();
    }
        processor->process(inFile, outFile, key);
}

void EncryptionUtils::AesEncryptor::process(std::ifstream &inFile, std::ofstream &outFile, const std::string &key) {
    EncryptionUtils::encryptDecrypt(inFile, outFile, key, true);
}

void EncryptionUtils::AesDecryptor::process(std::ifstream &inFile, std::ofstream &outFile, const std::string &key) {
    EncryptionUtils::encryptDecrypt(inFile, outFile, key, false);
}

std::string EncryptionUtils::encryptString(const std::string &input, const std::string &key) {
    std::vector<unsigned char> iv(16);
    if (!generateIv(iv.data(), iv.size())) {
        throw std::runtime_error("Failed to generate IV for string encryption");
    }

    std::vector<unsigned char> ciphertext(input.size() + EVP_MAX_BLOCK_LENGTH);
    int len;
    int ciphertext_len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv.data())) handleErrors();

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(input.c_str()), input.size())) handleErrors();
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) handleErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    std::string encryptedString(reinterpret_cast<char*>(iv.data()), iv.size());
    encryptedString += std::string(reinterpret_cast<char*>(ciphertext.data()), ciphertext_len);
    return encryptedString;
}

std::string EncryptionUtils::decryptString(const std::string &input, const std::string &key) {
    std::vector<unsigned char> iv(16);
    std::copy(input.begin(), input.begin() + iv.size(), iv.begin());

    std::vector<unsigned char> ciphertext(input.begin() + iv.size(), input.end());
    std::vector<unsigned char> decryptedtext(ciphertext.size());
    int len;
    int decrypted_len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*>(key.c_str()), iv.data())) handleErrors();

    if (1 != EVP_DecryptUpdate(ctx, decryptedtext.data(), &len, ciphertext.data(), ciphertext.size())) handleErrors();
    decrypted_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, decryptedtext.data() + len, &len)) handleErrors();
    decrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return std::string(reinterpret_cast<char*>(decryptedtext.data()), decrypted_len);
}

std::string EncryptionUtils::generateSignature(const std::string &data, const std::string &privateKey) {
    EVP_PKEY *pkey = nullptr;
    BIO *bio = BIO_new_mem_buf(privateKey.c_str(), -1);
    PEM_read_bio_PrivateKey(bio, &pkey, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
        Logger::log(Logger::ERROR, "Failed to load private key");
        throw std::runtime_error("Failed to load private key");
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) handleErrors();

    if (1 != EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey)) handleErrors();

    if (1 != EVP_DigestSignUpdate(ctx, data.c_str(), data.size())) handleErrors();

    size_t sigLen = 0;
    if (1 != EVP_DigestSignFinal(ctx, nullptr, &sigLen)) handleErrors();

    std::vector<unsigned char> sig(sigLen);
    if (1 != EVP_DigestSignFinal(ctx, sig.data(), &sigLen)) handleErrors();

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    return std::string(reinterpret_cast<char*>(sig.data()), sigLen);
}

bool EncryptionUtils::verifySignature(const std::string &data, const std::string &signature, const std::string &publicKey) {
    EVP_PKEY *pkey = nullptr;
    BIO *bio = BIO_new_mem_buf(publicKey.c_str(), -1);
    PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr);
    BIO_free(bio);

    if (!pkey) {
        Logger::log(Logger::ERROR, "Failed to load public key");
        throw std::runtime_error("Failed to load public key");
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) handleErrors();

    if (1 != EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey)) handleErrors();

    if (1 != EVP_DigestVerifyUpdate(ctx, data.c_str(), data.size())) handleErrors();

    int result = EVP_DigestVerifyFinal(ctx, reinterpret_cast<const unsigned char*>(signature.c_str()), signature.size());

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    if (result != 1) {
        handleErrors();
        return false;
    }

    return true;
}

std::vector<unsigned char> EncryptionUtils::deriveKey(const std::string &password, const std::vector<unsigned char> &salt, int iterations, size_t keyLength) {
    std::vector<unsigned char> key(keyLength);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(), iterations, EVP_sha256(), keyLength, key.data())) {
        Logger::log(Logger::ERROR, "Failed to derive key using PBKDF2");
        throw std::runtime_error("Failed to derive key using PBKDF2");
    }
    return key;
}

void EncryptionUtils::secureDelete(const std::string &filePath) {
    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs) {
        Logger::log(Logger::ERROR, "Failed to open file for secure deletion: " + filePath);
        throw std::runtime_error("Failed to open file for secure deletion: " + filePath);
    }

    ofs.seekp(0, std::ios::end);
    size_t length = ofs.tellp();
    ofs.seekp(0, std::ios::beg);

    std::vector<char> overwriteData(length, 0);
    ofs.write(overwriteData.data(), overwriteData.size());
    ofs.close();

    if (std::remove(filePath.c_str()) != 0) {
        Logger::log(Logger::ERROR, "Failed to delete file: " + filePath);
        throw std::runtime_error("Failed to delete file: " + filePath);
    }
}



