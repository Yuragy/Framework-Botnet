#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <future>
#include <openssl/ssl.h>
#include <openssl/err.h>

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool connect(const std::string &address, int port, int timeoutSeconds = 10, bool useSSL = false);
    void disconnect();
    bool sendData(const std::string &data);
    std::string receiveData();
    std::future<bool> sendDataAsync(const std::string &data);
    std::future<std::string> receiveDataAsync();
    bool isConnected() const;

    static void sendDataToServer(const std::string &data);
    static std::string getCommandFromServer();

private:
    int socket_fd;
    std::string serverAddress;
    int serverPort;
    bool sslEnabled;
    SSL_CTX* sslContext;
    SSL* ssl;

    void handleError(const std::string &errorMessage) const;
    void setSocketTimeout(int timeoutSeconds);
    bool initSSL();
    void cleanupSSL();
    bool establishConnection(const std::string &address, int port);
};

#endif // NETWORKMANAGER_H





