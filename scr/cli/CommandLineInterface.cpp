#include "CommandLineInterface.h"
#include "Logger.h"
#include <iostream>
#include <sstream>
#include <algorithm>

void CommandLineInterface::start() {
    Logger::log(Logger::INFO, "Starting command line interface");
    initializeCommands();
    prompt = "> ";
    autocompleteEnabled = true;

    std::string input;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);
        if (input == "exit") {
            Logger::log(Logger::INFO, "Exiting command line interface");
            break;
        }
        if (autocompleteEnabled) {
            input = autocompleteCommand(input);
        }
        executeCommand(input);
    }
}

void CommandLineInterface::executeCommand(const std::string &command) {
    auto parsed = parseCommand(command);
    auto it = commands.find(parsed.first);
    if (it != commands.end()) {
        try {
            it->second(parsed.second);
            Logger::log(Logger::INFO, "Executed command: " + parsed.first);
        } catch (const std::exception &e) {
            Logger::log(Logger::ERROR, "Error executing command: " + parsed.first + " - " + e.what());
        }
    } else {
        std::cout << "Unknown command: " << parsed.first << std::endl;
        Logger::log(Logger::WARNING, "Unknown command: " + parsed.first);
    }
}

void CommandLineInterface::registerCommand(const std::string &name, const std::function<void(const std::vector<std::string>&)> &func) {
    commands[name] = func;
}

void CommandLineInterface::registerCommand(const std::string &name, const std::function<void(const std::vector<std::string>&)> &func, const std::string &description) {
    commands[name] = func;
    commandDescriptions[name] = description;
}

void CommandLineInterface::initializeCommands() {
    registerCommand("help", [this](const std::vector<std::string>&) {
        std::cout << "Available commands:" << std::endl;
        for (const auto &cmd : commandDescriptions) {
            std::cout << " - " << cmd.first << ": " << cmd.second << std::endl;
        }
    }, "Show this help message");

    registerCommand("version", [](const std::vector<std::string>&) {
        std::cout << "Version 1.0" << std::endl;
    }, "Show the version of the program");

    registerCommand("echo", [](const std::vector<std::string>& args) {
        for (const auto &arg : args) {
            std::cout << arg << " ";
        }
        std::cout << std::endl;
    }, "Echo the input arguments");
}

std::pair<std::string, std::vector<std::string>> CommandLineInterface::parseCommand(const std::string &input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    std::string arg;
    std::vector<std::string> args;
    while (iss >> arg) {
        args.push_back(arg);
    }
    return {command, args};
}

std::string CommandLineInterface::getCommandDescription(const std::string &name) const {
    auto it = commandDescriptions.find(name);
    if (it != commandDescriptions.end()) {
        return it->second;
    }
    return "No description available.";
}

std::vector<std::string> CommandLineInterface::getAllCommands() const {
    std::vector<std::string> commandList;
    for (const auto &cmd : commandDescriptions) {
        commandList.push_back(cmd.first + ": " + cmd.second);
    }
    return commandList;
}

void CommandLineInterface::setPrompt(const std::string &newPrompt) {
    prompt = newPrompt;
}

void CommandLineInterface::enableAutocomplete(bool enable) {
    autocompleteEnabled = enable;
}

std::string CommandLineInterface::autocompleteCommand(const std::string &input) {
    auto it = std::find_if(commands.begin(), commands.end(), [&input](const auto &cmd) {
        return cmd.first.find(input) == 0;
    });
    return it != commands.end() ? it->first : input;
}


