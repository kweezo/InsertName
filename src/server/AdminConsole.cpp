#include "AdminConsole.hpp"

#include "Config.hpp"
#include "Server.hpp"
#include "Log.hpp"

#include <algorithm>
#include <cstring>
#include <thread>


bool AdminConsole::isRunning = true;
std::vector<std::string> AdminConsole::commands;
WINDOW* AdminConsole::logWindow = nullptr;
WINDOW* AdminConsole::separatorWindow = nullptr;
WINDOW* AdminConsole::commandWindow = nullptr;
WINDOW* AdminConsole::commandFeedbackWindow = nullptr;
std::deque<std::string> AdminConsole::commandHistory;
int AdminConsole::currentCommand = -1;
std::string AdminConsole::prompt;
char AdminConsole::line[256] = {0};
int AdminConsole::commandWindowHeight;

void AdminConsole::init() {
    loadVariables();
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    noecho();
    initColors();
    addCommands();
    initWindows();
}

void AdminConsole::loadVariables() {
    commandWindowHeight = Config::GetInstance().commandWindowHeight;
    prompt = Config::GetInstance().commandPrefix;
}

void AdminConsole::initColors() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
}

void AdminConsole::initWindows() {
    logWindow = derwin(stdscr, LINES-commandWindowHeight-1, COLS, 0, 0); // Create a new window. Parameters: parent window, number of lines, number of columns, y position, x position
    scrollok(logWindow, TRUE); // Enable scrolling for the log window

    separatorWindow = derwin(stdscr, 1, COLS, LINES-commandWindowHeight-1, 0);

    commandFeedbackWindow = derwin(stdscr, commandWindowHeight-1, COLS, LINES-commandWindowHeight, 0);
    scrollok(commandFeedbackWindow, TRUE);

    commandWindow = derwin(stdscr, 1, COLS, LINES-1, 0);
    mvwhline(separatorWindow, 0, 0, ACS_HLINE, COLS); // Draw a horizontal line
    wrefresh(separatorWindow); // Refresh the separator window

}

void AdminConsole::addCommands() {
    commands.push_back("stop");
}

std::string AdminConsole::readLine() {
    move(LINES-1, 0); // Move the cursor to the command line
    clrtoeol(); // Clear the command line
    printw(prompt.c_str()); // Print the command prefix
    wrefresh(commandWindow);
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != '\n' && ch != '\r') {
        if (ch == '\t') {
            std::string currentInput(line);
            std::vector<std::string> matches;

            // Find all commands that start with the current input
            for (const auto& cmd : commands) {
                if (cmd.find(currentInput) == 0) {
                    matches.push_back(cmd);
                }
            }

            if (!matches.empty()) {
                // If there is only one match, complete the command
                if (matches.size() == 1) {
                    strncpy(line, matches[0].c_str(), sizeof(line) - 1);
                    pos = strlen(line);
                } else {
                    // Find the common prefix of all matches
                    std::string commonPrefix = matches[0];
                    for (const auto& match : matches) {
                        for (size_t i = 0; i < commonPrefix.size(); i++) {
                            if (i == match.size() || match[i] != commonPrefix[i]) {
                                commonPrefix = commonPrefix.substr(0, i);
                                break;
                            }
                        }
                    }
                    
                    // Copy the common prefix to the input line
                    strncpy(line, commonPrefix.c_str(), sizeof(line) - 1);
                    pos = strlen(line);
                }
            }

            move(LINES-1, 0);
            clrtoeol();
            printw(prompt.c_str());
            printw(line);
            move(LINES-1, pos + prompt.size());

        } else if (ch == KEY_UP || ch == KEY_DOWN) {
            processKey(ch, prompt);
            pos = strlen(line);

        } else {
            if (ch == 8 || ch == 127) {
                if (pos > 0) {
                    pos--;
                    line[pos] = '\0';
                    currentCommand = -1;
                }

            } else if (ch >= 32 && ch <= 126 && pos < COLS - prompt.size() - 1) {
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
    line[0] = '\0';

    return strLine;
}

void AdminConsole::processKey(int key, const std::string& prompt) {
    if (key == KEY_UP) {
        if (!commandHistory.empty() && currentCommand < (int)commandHistory.size() - 1) {
            currentCommand++;
            strncpy(line, commandHistory[commandHistory.size() - 1 - currentCommand].c_str(), sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0'; // Add null terminator
        }
    } else if (key == KEY_DOWN) {
        if (currentCommand > 0) {
            currentCommand--;
            strncpy(line, commandHistory[commandHistory.size() - 1 - currentCommand].c_str(), sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0'; // Add null terminator
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

void AdminConsole::cmdReport(const std::string& msg, int colorPair) { // Same functionality as printLog, just different window
    wattron(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wprintw(commandFeedbackWindow, "\n%s", msg.c_str());
    wattroff(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wrefresh(commandFeedbackWindow);
    wmove(commandWindow, 0, strlen(line) + prompt.size());
    wrefresh(commandWindow);
}

void AdminConsole::printLog(const std::string& msg, int colorPair) {
    // Set the color pair for the log window
    wattron(logWindow, COLOR_PAIR(colorPair));
    // Print the message in the log window
    wprintw(logWindow, "\n%s", msg.c_str());
    // Turn off the color pair
    wattroff(logWindow, COLOR_PAIR(colorPair));
    wrefresh(logWindow);

    // Move the cursor back to the command line
    wmove(commandWindow, 0, strlen(line) + prompt.size());
    wrefresh(commandWindow);
}

void AdminConsole::processLine(const std::string& line) {
    if (line.empty() || !isRunning) {
        return;
    }

    std::vector<std::string> commands;
    std::string temp;
    bool inQuotes = false;

    for (char ch : line) {
        if (ch == '"' && inQuotes) {
            // End of quotes
            inQuotes = false;
        } else if (ch == '"' && !inQuotes) {
            // Start of quotes
            inQuotes = true;
        } else if (ch == ' ' && !inQuotes) {
            // Space outside of quotes, add argument to the list
            if (!temp.empty()) {
                commands.push_back(temp);
                temp.clear();
            }
        } else {
            // Add character to the current argument
            temp += ch;
        }
    }

    // Add the last argument to the list, if it exists
    if (!temp.empty()) {
        commands.push_back(temp);
    }

    if (!commands.empty()) {
        int cmdSize = commands.size();

        if (commands[0] == "stop") {
            double value;
            if (cmdSize == 2) {
                if (isDouble(commands[1], value)) {
                    cmdStop(value);
                } else {
                    cmdReport("Invalid argument for stop command. Argument should be type double.", 3);
                }
            } else {
                if (cmdSize == 1) {
                    cmdReport("Stop command requires a time argument (double)", 3);
                } else if (cmdSize > 2) {
                    cmdReport("Too much arguments for stop command. It takes only a time argument (double)", 3);
                }
            }

        } else {
            cmdReport("Unknown command: " + line, 4);
        }
    }
}

void AdminConsole::cmdStop(double waitTime) {
    cmdReport("Stopping server in " + std::to_string(waitTime) + " minutes...", 2);
    Server::isShuttingDown = true;

    std::thread([waitTime]() {
        int waitTimeInSeconds = static_cast<int>(waitTime * 60);
        std::this_thread::sleep_for(std::chrono::seconds(waitTimeInSeconds));

        cmdReport("Stopping server...", 2);
        Log::print(1, "Server stopped by admin command");
        Server::shutdown = true;

        Server::stop();
        Log::destroy();
        isRunning = false;

        cmdReport("Server stopped. Press ENTER to exit", 1);
    }).detach();
}

bool AdminConsole::isDouble(const std::string& s, double& d) {
    std::istringstream iss(s);
    iss >> d;
    return iss.eof() && !iss.fail();
}