#include "Socket.h"
#include "Logger.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>
#include <netdb.h>

Socket::Socket() 
    : socket_fd(-1), serverPort(0), sslEnabled(false), sslContext(nullptr), ssl(nullptr) {}

Socket::~Socket() {
    disconnect();
    cleanupSSL();
}

bool Socket::connect(const std::string &address, int port, int timeoutSeconds, bool useSSL, bool useIPv6) {
    try {
        serverAddress = address;
        serverPort = port;
        sslEnabled = useSSL;

        if (useSSL && !initSSL()) {
            return false;
        }

        socket_fd = socket(useIPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            handleError("Failed to create socket");
            return false;
        }

        setSocketTimeout(timeoutSeconds);

        sockaddr_in server_addr;
        sockaddr_in6 server_addr6;

        if (useIPv6) {
            memset(&server_addr6, 0, sizeof(server_addr6));
            server_addr6.sin6_family = AF_INET6;
            server_addr6.sin6_port = htons(port);

            if (inet_pton(AF_INET6, address.c_str(), &server_addr6.sin6_addr) <= 0) {
                handleError("Invalid IPv6 address");
                return false;
            }

            if (::connect(socket_fd, (struct sockaddr *)&server_addr6, sizeof(server_addr6)) < 0) {
                handleError("Connection failed");
                return false;
            }
        } else {
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, address.c_str(), &server_addr.sin_addr) <= 0) {
                handleError("Invalid IPv4 address");
                return false;
            }

            if (::connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                handleError("Connection failed");
                return false;
            }
        }

        if (useSSL) {
            ssl = SSL_new(sslContext);
            SSL_set_fd(ssl, socket_fd);

            if (SSL_connect(ssl) <= 0) {
                handleError("SSL connection failed");
                return false;
            }
        }

        Logger::log(Logger::INFO, "Connected to " + address + ":" + std::to_string(port));
        return true;
    } catch (const std::exception &e) {
        handleError("Exception in connect: " + std::string(e.what()));
        return false;
    }
}

void Socket::disconnect() {
    std::lock_guard<std::mutex> lock(socketMutex);
    try {
        if (sslEnabled && ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }

        if (socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
            Logger::log(Logger::INFO, "Disconnected from server");
        }
    } catch (const std::exception &e) {
        handleError("Exception in disconnect: " + std::string(e.what()));
    }
}

bool Socket::sendData(const std::string &data) {
    std::lock_guard<std::mutex> lock(socketMutex);
    try {
        if (socket_fd < 0) {
            handleError("Not connected to server");
            return false;
        }

        ssize_t bytesSent;
        if (sslEnabled) {
            bytesSent = SSL_write(ssl, data.c_str(), data.size());
        } else {
            bytesSent = send(socket_fd, data.c_str(), data.size(), 0);
        }

        if (bytesSent < 0) {
            handleError("Failed to send data");
            return false;
        }

        Logger::log(Logger::INFO, "Sent " + std::to_string(bytesSent) + " bytes");
        return true;
    } catch (const std::exception &e) {
        handleError("Exception in sendData: " + std::string(e.what()));
        return false;
    }
}

std::string Socket::receiveData() {
    std::lock_guard<std::mutex> lock(socketMutex);
    try {
        if (socket_fd < 0) {
            handleError("Not connected to server");
            throw std::runtime_error("Not connected to server");
        }

        char buffer[1024] = {0};
        ssize_t bytesReceived;
        if (sslEnabled) {
            bytesReceived = SSL_read(ssl, buffer, sizeof(buffer));
        } else {
            bytesReceived = recv(socket_fd, buffer, sizeof(buffer), 0);
        }

        if (bytesReceived < 0) {
            handleError("Failed to receive data");
            throw std::runtime_error("Failed to receive data");
        }

        Logger::log(Logger::INFO, "Received " + std::to_string(bytesReceived) + " bytes");
        return std::string(buffer, bytesReceived);
    } catch (const std::exception &e) {
        handleError("Exception in receiveData: " + std::string(e.what()));
        throw;
    }
}

std::future<bool> Socket::sendDataAsync(const std::string &data) {
    return std::async(std::launch::async, [this, data]() {
        return this->sendData(data);
    });
}

std::future<std::string> Socket::receiveDataAsync() {
    return std::async(std::launch::async, [this]() {
        return this->receiveData();
    });
}

bool Socket::isConnected() const {
    std::lock_guard<std::mutex> lock(socketMutex);
    return socket_fd >= 0;
}

void Socket::handleError(const std::string &errorMessage) const {
    Logger::log(Logger::ERROR, "Socket error: " + errorMessage);
}

void Socket::setSocketTimeout(int timeoutSeconds) {
    timeval timeout;
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        handleError("Failed to set receive timeout");
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        handleError("Failed to set send timeout");
    }
}

bool Socket::initSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    sslContext = SSL_CTX_new(TLS_client_method());
    if (!sslContext) {
        handleError("Failed to create SSL context");
        return false;
    }

    return true;
}

void Socket::cleanupSSL() {
    if (sslContext) {
        SSL_CTX_free(sslContext);
        sslContext = nullptr;
    }

    EVP_cleanup();
}


