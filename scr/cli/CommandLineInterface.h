#ifndef COMMANDLINEINTERFACE_H
#define COMMANDLINEINTERFACE_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class CommandLineInterface {
public:

    void start();
    void executeCommand(const std::string &command);
    void registerCommand(const std::string &name, const std::function<void(const std::vector<std::string>&)> &func);
    void registerCommand(const std::string &name, const std::function<void(const std::vector<std::string>&)> &func, const std::string &description);

    std::pair<std::string, std::vector<std::string>> parseCommand(const std::string &input);
    std::string getCommandDescription(const std::string &name) const;
    std::vector<std::string> getAllCommands() const;

    void setPrompt(const std::string &prompt);
    void enableAutocomplete(bool enable);

private:
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    std::unordered_map<std::string, std::string> commandDescriptions;
    std::string prompt;
    bool autocompleteEnabled;

    void initializeCommands();

    std::string autocompleteCommand(const std::string &input);
};

#endif // COMMANDLINEINTERFACE_H


