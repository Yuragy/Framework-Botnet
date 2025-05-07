#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <future>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <mutex>

class Socket {
public:
    Socket();
    ~Socket();
    bool connect(const std::string &address, int port, int timeoutSeconds = 10, bool useSSL = false, bool useIPv6 = false);
    void disconnect();
    bool sendData(const std::string &data);
    std::string receiveData();
    std::future<bool> sendDataAsync(const std::string &data);
    std::future<std::string> receiveDataAsync();
    bool isConnected() const;

private:
    int socket_fd;
    std::string serverAddress;
    int serverPort;
    bool sslEnabled;
    SSL_CTX* sslContext;
    SSL* ssl;
    std::mutex socketMutex;

    void handleError(const std::string &errorMessage) const;
    void setSocketTimeout(int timeoutSeconds);
    bool initSSL();
    void cleanupSSL();
};

#endif // SOCKET_H


