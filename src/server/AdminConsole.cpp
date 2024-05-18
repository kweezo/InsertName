#include "AdminConsole.hpp"

#include <algorithm>
#include <cstring>


std::vector<std::string> AdminConsole::commands;
WINDOW* AdminConsole::logWindow = nullptr;
WINDOW* AdminConsole::commandWindow = nullptr;
std::deque<std::string> AdminConsole::commandHistory;
int AdminConsole::currentCommand = -1;
char AdminConsole::line[256] = {0};

void AdminConsole::init() {
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    noecho();
    addCommands();
    initWindows();
}

void AdminConsole::initWindows() {
    logWindow = newwin(LINES - 1, COLS, 0, 0);
    commandWindow = newwin(1, COLS, LINES - 1, 0);
}

void AdminConsole::addCommands() {
    commands.push_back("stop");
}

std::string AdminConsole::readLine(const std::string& prompt) {
    move(LINES-1, 0);
    clrtoeol();
    printw(prompt.c_str());
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != '\n' && ch != '\r') {
        if (ch == KEY_UP || ch == KEY_DOWN) {
            processKey(ch, prompt);
            pos = strlen(line);
        } else {
            if (ch == 8 || ch == 127) {
                if (pos > 0) {
                    pos--;
                    line[pos] = '\0';
                    currentCommand = -1;
                }
            } else if (ch >= 32 && ch <= 126) {
                line[pos++] = ch;
                line[pos] = '\0';
                currentCommand = -1;
            }
            move(LINES-1, 0);
            clrtoeol();
            printw(prompt.c_str());
            printw(line);
            move(LINES-1, pos + prompt.size());  // Move the cursor to the correct position
        }
        if (pos == 0 && (ch == '\n' || ch == '\r')) {
            return "";  // Return an empty string if the line is empty
        }
    }
    std::string strLine(line);
    if (!strLine.empty()) {
        // Add the command to the history
        commandHistory.push_back(strLine);
        currentCommand = -1;
    }
    return strLine;
}

void AdminConsole::processKey(int key, const std::string& prompt) {
    if (key == KEY_UP) {
        if (!commandHistory.empty() && currentCommand < (int)commandHistory.size() - 1) {
            currentCommand++;
            strcpy(line, commandHistory[commandHistory.size() - 1 - currentCommand].c_str());
        }
    } else if (key == KEY_DOWN) {
        if (currentCommand > 0) {
            currentCommand--;
            strcpy(line, commandHistory[commandHistory.size() - 1 - currentCommand].c_str());
        } else if (currentCommand == 0) {
            currentCommand = -1;
            line[0] = '\0';
        }
    }
    // Update the console
    int pos = strlen(line);
    move(LINES-1, 0);
    clrtoeol();
    printw(prompt.c_str());
    printw(line);
    move(LINES-1, pos + prompt.size());  // Move the cursor to the correct position
}

void AdminConsole::printLog(const std::string& msg, int colorPair) {
    // Set the color pair for the log window
    wattron(logWindow, COLOR_PAIR(colorPair));
    // Print the message in the log window
    wprintw(logWindow, "%s\n", msg.c_str());
    // Turn off the color pair
    wattroff(logWindow, COLOR_PAIR(colorPair));
    wrefresh(logWindow);

    // Move the cursor back to the command line
    move(LINES-1, strlen(line) + 2);  // +2 to account for the "> " prompt
}

std::string AdminConsole::readCommand() {
    char line[256];
    wgetstr(commandWindow, line);
    return std::string(line);
}

void AdminConsole::processLine(const std::string& line) {
    if (line.empty()) {
        return;
    }
    if (line == "stop") {
        cmdStop();
    } else {
        printw("Unknown command: %s\n", line.c_str());
    }
}

void AdminConsole::cmdStop() {
    endwin(); // End PDCurses session
    exit(0);
}