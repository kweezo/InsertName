#include "AdminConsole.hpp"
#include <algorithm>

// Initialize the static member variable
AdminConsole* AdminConsole::currentInstance = nullptr;

AdminConsole::AdminConsole() {
    currentInstance = this;
    linenoiseSetCompletionCallback(completionCallback);
}

void AdminConsole::addCommand(const std::string& command) {
    commands.push_back(command);
}

std::string AdminConsole::readLine(const std::string& prompt) {
    char* line = linenoise(prompt.c_str());
    if (line) {
        linenoiseHistoryAdd(line);
        std::string lineStr(line);
        free(line); // use free instead of linenoiseFree
        return lineStr;
    }
    return "";
}

void AdminConsole::completionCallback(const char* editBuffer, linenoiseCompletions* lc) {
    for (const auto& command : currentInstance->commands) {
        if (command.find(editBuffer) == 0) {
            linenoiseAddCompletion(lc, command.c_str());
        }
    }
}