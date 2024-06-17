#include "AdminConsole.hpp"

#include "Config.hpp"
#include "Server.hpp"
#include "Log.hpp"

#include <algorithm>
#include <cstring>
#include <thread>
#include <regex>


bool AdminConsole::isRunning = true;
std::array<std::string, COMMAND_COUNT> AdminConsole::commands;
std::array<std::vector<std::string>, COMMAND_COUNT> AdminConsole::secParam;
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
    commandWindowHeight = Config::commandWindowHeight;
    prompt = Config::commandPrefix;
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
    commands[0]="stop";
    commands[1]="config";

    secParam[0] = {};
    secParam[1] = {"dbname", "dbuser", "dbpassword", "dbhostaddr", "dbport", "serverPort", "loginAttempts", "logLevel", "maxLogBufferSize", "commandPrefix", "commandWindowHeight"};
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
            size_t spacePos = currentInput.find(' ');
            int commandIndex = -1;

            if (spacePos != std::string::npos) {
                std::string baseCommand = currentInput.substr(0, spacePos);
                std::string additionalParam = currentInput.substr(spacePos + 1);

                // Find the index of the base command
                for (size_t i = 0; i < commands.size(); ++i) {
                    if (commands[i] == baseCommand) {
                        commandIndex = i;
                        break;
                    }
                }

                if (commandIndex != -1) {
                    for (const auto& param : secParam[commandIndex]) {
                        if (param.find(additionalParam) == 0) {
                            matches.push_back(param);
                        }
                    }
                } else {
                    continue;
                }

            } else {
                // Find all commands that start with the current input
                for (const auto& cmd : commands) {
                    if (cmd.find(currentInput) == 0) {
                        matches.push_back(cmd);
                    }
                }
            }

            if (!matches.empty()) {
                // If there is only one match, complete the command
                if (matches.size() == 1) {
                    strncpy(line, (((spacePos != std::string::npos && commandIndex != -1) ? commands[commandIndex] + ' ' : "") + matches[0] + ' ').c_str(), sizeof(line) - 1);
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
                    strncpy(line, (((spacePos != std::string::npos && commandIndex != -1) ? commands[commandIndex] + ' ' : "") + commonPrefix).c_str(), sizeof(line) - 1);
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
        if (ch == '\'' && inQuotes) {
            // End of quotes
            inQuotes = false;
        } else if (ch == '\'' && !inQuotes) {
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
                    cmdReport("Invalid argument for 'stop' command. Argument should be type double.", 4);
                }
            } else {
                if (cmdSize == 1) {
                    cmdReport("'stop' command requires a time argument.", 4);
                } else {
                    cmdReport("Too much arguments for 'stop' command.", 4);
                }
            }

        } else if (commands[0] == "config") {
            if (cmdSize == 1) {
                cmdReport("'config' command requires a parameters", 4);
            } else {
                auto it = std::find(secParam[1].begin(), secParam[1].end(), commands[1]);
                int index;
                if (it != secParam[1].end()) {
                    index = std::distance(secParam[1].begin(), it);
                } else {
                    index = -1;
                }

                if (cmdSize == 2) {
                    if (index == -1) {
                        cmdReport("Unknown config parameter: " + commands[1], 4);
                    } else {
                        cmdReport("Setting '" + commands[1] + "' is set to: '" + *static_cast<std::string*>(Config::configPointers[index]) + '\'', 2);
                    }

                } else  if (cmdSize == 3) {
                    if (index == -1) {
                        cmdReport("Unknown config parameter: " + commands[1], 4);
                    } else {
                        if (index <= 2 || index == 9) {
                            std::string* stringPointer = static_cast<std::string*>(Config::configPointers[index]);
                            *stringPointer = commands[2];
                        } else if (index == 3) {
                            if (isValidIPv4(commands[2])) {
                                std::string* stringPointer = static_cast<std::string*>(Config::configPointers[index]);
                                *stringPointer = commands[2];
                            } else {
                                cmdReport("Invalid IPv4 address", 4);
                            }
                        } else {
                            int value;
                            if (isInt(commands[2], value)) {
                                if (index == 4 && (value < 1 || value > 65535)) {
                                    cmdReport("Database port must be greater than 0 and smaller than 65536", 4);
                                } else if (index == 5 && (value < 1 || value > 65535)) {
                                    cmdReport("Server port must be greater than  and smaller than 65536", 4);
                                } else if (index == 6 && value < 1) {
                                    cmdReport("Login attempts must be greater than 0", 4);
                                } else if (index == 7 && (value < 0 || value > 4)) {
                                    cmdReport("Log level must be between 0 and 4", 4);
                                } else if (index == 8 && value < 1) {
                                    cmdReport("Max log buffer size must be greater than 0", 4);
                                } else if (index == 10 && value < 1) {
                                    cmdReport("Command window height must be greater than 0", 4);
                                } else {
                                    int* intPointer = static_cast<int*>(Config::configPointers[index]);
                                    *intPointer = static_cast<int>(value);
                                }
                            } else {
                                cmdReport("Invalid argument for config command. Argument should be type int", 4);
                            }
                        }
                    }
                } else {
                    cmdReport("Too much arguments for config command", 4);
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

        Config::SaveConfig();
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

bool AdminConsole::isInt(const std::string& s, int& i) {
    std::istringstream iss(s);
    iss >> i;
    return iss.eof() && !iss.fail();
}

bool AdminConsole::isValidIPv4(const std::string& ip) {
    std::regex ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if (std::regex_match(ip, ipRegex)) {
        return true;
    }
    return false;
}