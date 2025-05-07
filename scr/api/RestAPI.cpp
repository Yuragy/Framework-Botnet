#include "RestAPI.h"
#include "Logger.h"
#include <stdexcept>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <boost/asio/ssl.hpp>
#include <jwt-cpp/jwt.h>

RestAPI::RestAPI()
    : serverRunning(false),
      serverPort(0),
      sslContext(std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12))
{}

RestAPI::~RestAPI() {
    if (serverRunning) {
        stopServer();
    }
}

void RestAPI::startServer(int port) {
    if (serverRunning) {
        Logger::log(Logger::ERROR, "Server is already running");
        throw std::runtime_error("Server is already running");
    }
    serverPort = port;
    serverRunning = true;
    serverThread = std::thread(&RestAPI::runServer, this);
    Logger::log(Logger::INFO, "Starting server on port " + std::to_string(port));
}

void RestAPI::stopServer() {
    if (!serverRunning) {
        Logger::log(Logger::ERROR, "Server is not running");
        throw std::runtime_error("Server is not running");
    }
    serverRunning = false;
    if (serverThread.joinable()) {
        serverThread.join();
    }
    Logger::log(Logger::INFO, "Stopping server");
}

void RestAPI::runServer() {
    boost::asio::io_context io_context;

    boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverPort));

    while (serverRunning) {
        boost::asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);

        boost::asio::streambuf request_buf;
        boost::asio::read_until(socket, request_buf, "\r\n");

        std::istream request_stream(&request_buf);
        std::string request;
        std::getline(request_stream, request);

        std::string response = handleRequest(request);

        boost::asio::write(socket, boost::asio::buffer(response));
    }
}

std::string RestAPI::handleRequest(const std::string &request) {
    if (!serverRunning) {
        Logger::log(Logger::ERROR, "Server is not running");
        throw std::runtime_error("Server is not running");
    }

    logRequest(request);
    
    // Extract token for authentication
    std::string token = "extracted_token"; // Placeholder extraction logic
    authenticateRequest(token);

    if (request.find("GET") == 0) {
        return handleGetRequest(request);
    } else if (request.find("POST") == 0) {
        return handlePostRequest(request);
    } else {
        Logger::log(Logger::ERROR, "Unsupported request method");
        return "Unsupported request method";
    }
}

void RestAPI::addRoute(const std::string &path, const std::function<std::string(const std::string&)>& handler) {
    routes[path] = handler;
}

std::string RestAPI::handleGetRequest(const std::string &request) {
    std::istringstream requestStream(request);
    std::string method, path;
    requestStream >> method >> path;

    auto it = routes.find(path);
    if (it != routes.end()) {
        return it->second(request);
    } else {
        return "404 Not Found";
    }
}

std::string RestAPI::handlePostRequest(const std::string &request) {
    std::istringstream requestStream(request);
    std::string method, path;
    requestStream >> method >> path;

    auto it = routes.find(path);
    if (it != routes.end()) {
        return it->second(request);
    } else {
        return "404 Not Found";
    }
}

void RestAPI::logRequest(const std::string &request) {
    Logger::log(Logger::INFO, "Handling request: " + request);
}

void RestAPI::startSecureServer(int port) {
    // Implement starting a secure server
}

void RestAPI::stopSecureServer() {
    // Implement stopping a secure server
}

void RestAPI::configureSSL(const std::string &certFile, const std::string &keyFile) {
    sslContext->use_certificate_chain_file(certFile);
    sslContext->use_private_key_file(keyFile, boost::asio::ssl::context::pem);
}

void RestAPI::authenticateRequest(const std::string &token) {
    auto decoded = jwt::decode(token);
    // Validate token and set user context
}









