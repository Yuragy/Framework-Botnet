#include "Encryption.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>

void Encryption::generateKeyAndIV(const std::string &key, unsigned char *outKey, unsigned char *outIV, const std::string &algorithm) {
    int keySize, ivSize;
    getKeyAndIVSizes(algorithm, keySize, ivSize);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) handleOpenSSLErrors();

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr)) handleOpenSSLErrors();
    if (1 != EVP_DigestUpdate(mdctx, key.c_str(), key.length())) handleOpenSSLErrors();
    if (1 != EVP_DigestFinal_ex(mdctx, hash, nullptr)) handleOpenSSLErrors();
    EVP_MD_CTX_free(mdctx);

    std::memcpy(outKey, hash, keySize);
    std::memcpy(outIV, hash + keySize, ivSize);
}

void Encryption::handleOpenSSLErrors() {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error("OpenSSL error occurred");
}

std::vector<unsigned char> Encryption::stringToBytes(const std::string &str) {
    return std::vector<unsigned char>(str.begin(), str.end());
}

std::string Encryption::bytesToString(const std::vector<unsigned char> &bytes) {
    return std::string(bytes.begin(), bytes.end());
}

void Encryption::getKeyAndIVSizes(const std::string &algorithm, int &keySize, int &ivSize) {
    if (algorithm == "aes-256-cbc") {
        keySize = 32;
        ivSize = 16;
            } else if (algorithm == "aes-192-cbc") {
        keySize = 24;
        ivSize = 16;
    } else if (algorithm == "aes-128-cbc") {
        keySize = 16;
        ivSize = 16;
    } else {
        throw std::invalid_argument("Unsupported encryption algorithm");
    }
}

std::string Encryption::encrypt(const std::string &data, const std::string &key, const std::string &algorithm) {
    int keySize, ivSize;
    getKeyAndIVSizes(algorithm, keySize, ivSize);

    unsigned char outKey[keySize];
    unsigned char outIV[ivSize];
    generateKeyAndIV(key, outKey, outIV, algorithm);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleOpenSSLErrors();

    const EVP_CIPHER *cipher = EVP_get_cipherbyname(algorithm.c_str());
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::invalid_argument("Unsupported encryption algorithm");
    }

    if (1 != EVP_EncryptInit_ex(ctx, cipher, nullptr, outKey, outIV)) handleOpenSSLErrors();

    std::vector<unsigned char> ciphertext(data.size() + EVP_CIPHER_block_size(cipher));
    int len = 0, ciphertext_len = 0;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char *>(data.c_str()), data.size())) handleOpenSSLErrors();
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) handleOpenSSLErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return bytesToString(std::vector<unsigned char>(ciphertext.begin(), ciphertext.begin() + ciphertext_len));
}

std::string Encryption::decrypt(const std::string &data, const std::string &key, const std::string &algorithm) {
    int keySize, ivSize;
    getKeyAndIVSizes(algorithm, keySize, ivSize);

    unsigned char outKey[keySize];
    unsigned char outIV[ivSize];
    generateKeyAndIV(key, outKey, outIV, algorithm);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleOpenSSLErrors();

    const EVP_CIPHER *cipher = EVP_get_cipherbyname(algorithm.c_str());
    if (!cipher) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::invalid_argument("Unsupported encryption algorithm");
    }

    if (1 != EVP_DecryptInit_ex(ctx, cipher, nullptr, outKey, outIV)) handleOpenSSLErrors();

    std::vector<unsigned char> plaintext(data.size());
    int len = 0, plaintext_len = 0;

    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, reinterpret_cast<const unsigned char *>(data.c_str()), data.size())) handleOpenSSLErrors();
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) handleOpenSSLErrors();
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return bytesToString(std::vector<unsigned char>(plaintext.begin(), plaintext.begin() + plaintext_len));
}



