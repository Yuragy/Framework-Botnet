#ifndef RESTAPI_H
#define RESTAPI_H

#include <string>
#include <thread>
#include <map>
#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class RestAPI {
public:
    RestAPI();
    ~RestAPI();

    void startServer(int port);
    void stopServer();
    std::string handleRequest(const std::string &request);

    void addRoute(const std::string &path, const std::function<std::string(const std::string&)>& handler);

private:
    bool serverRunning;
    int serverPort;
    std::thread serverThread;

    std::map<std::string, std::function<std::string(const std::string&)>> routes;

    void runServer();
    std::string handleGetRequest(const std::string &request);
    std::string handlePostRequest(const std::string &request);
    void logRequest(const std::string &request);

    void startSecureServer(int port);
    void stopSecureServer();
    std::unique_ptr<boost::asio::ssl::context> sslContext;
    void configureSSL(const std::string &certFile, const std::string &keyFile);

    void authenticateRequest(const std::string &token);
};

#endif // RESTAPI_H






