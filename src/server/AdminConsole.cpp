#include "AdminConsole.hpp"
#include <algorithm>
#include <curses.h> // Include PDCurses header

AdminConsole* AdminConsole::currentInstance = nullptr;

AdminConsole::AdminConsole() {
    currentInstance = this;
    initscr(); // Initialize PDCurses
    addCommands();
}

void AdminConsole::addCommands() {
    commands.push_back("stop");
}

std::string AdminConsole::readLine(const std::string& prompt) {
    if (currentInstance == nullptr) {
        return "";
    }

    printw(prompt.c_str()); // Use PDCurses printw instead of cout
    char line[256];
    getstr(line); // Use PDCurses getstr to read a line
    return std::string(line);
}

void AdminConsole::processLine(const std::string& line) {
    if (line == "stop") {
        cmdStop();
    } else {
        printw("Unknown command: %s\n", line.c_str()); // Use PDCurses printw instead of cout
    }
}

void AdminConsole::cmdStop() {
    currentInstance = nullptr;
    endwin(); // End PDCurses session
}