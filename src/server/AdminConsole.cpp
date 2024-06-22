#include "AdminConsole.hpp"

#include "Config.hpp"
#include "Server.hpp"
#include "Log.hpp"

#include <algorithm>
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
int AdminConsole::currentCommand;
std::string AdminConsole::prompt;
std::string AdminConsole::line;
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
    prompt = Config::commandPrompt;
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
    commands = {"stop", "config"};

    secParam[0] = {"0"};
    secParam[1] = {"dbname", "dbuser", "dbpassword", "dbhostaddr", "dbport", "serverport", "loginattempts", "loglevel", "maxlogbuffersize", "commandprompt", "commandwindowheight"};
}

std::string AdminConsole::readLine() {
    move(LINES-1, 0); // Move the cursor to the command line
    clrtoeol(); // Clear the command line
    printw(prompt.c_str()); // Print the command prefix
    wrefresh(commandWindow);

    int ch;
    std::string line;
    
    while ((ch = getch()) != '\n' && ch != '\r') {
        if (ch == '\t') {
            std::vector<std::string> matches;
            size_t spacePos = line.find(' ');
            int commandIndex = -1;

            if (spacePos != std::string::npos) {
                std::string baseCommand = line.substr(0, spacePos);
                std::string additionalParam = line.substr(spacePos + 1);

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
                    if (cmd.find(line) == 0) {
                        matches.push_back(cmd);
                    }
                }
            }

            if (!matches.empty()) {
                // If there is only one match, complete the command
                if (matches.size() == 1) {
                    line = ((spacePos != std::string::npos && commandIndex != -1) ? commands[commandIndex] + ' ' : "") + matches[0] + ' ';
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
                    line = ((spacePos != std::string::npos && commandIndex != -1) ? commands[commandIndex] + ' ' : "") + commonPrefix;
                }
            }

            move(LINES-1, 0);
            clrtoeol();
            printw(prompt.c_str());
            printw(line.c_str());
            move(LINES-1, line.length() + prompt.size());

        } else if (ch == KEY_UP || ch == KEY_DOWN) {
            processKey(ch, prompt);

        } else {
            if (ch == 8 || ch == 127) {
                if (!line.empty()) {
                    line.pop_back(); // Remove the last character
                    currentCommand = -1;
                }

            } else if (ch >= 32 && ch <= 126 && line.length() < COLS - prompt.size() - 1) {
                line += ch; // Append the character to the line
                currentCommand = -1;
            }

            move(LINES-1, 0);
            clrtoeol();
            printw(prompt.c_str());
            printw(line.c_str());
            move(LINES-1, line.length() + prompt.size());  // Move the cursor to the correct position
        }
        if (line.empty() && (ch == '\n' || ch == '\r')) {
            return "";  // Return an empty string if the line is empty
        }
    }

    if (!line.empty()) {
        // Add the command to the history
        commandHistory.push_back(line);
        currentCommand = -1;
    }

    return line;
}

void AdminConsole::processKey(int key, const std::string& prompt) {
    if (key == KEY_UP) {
        if (!commandHistory.empty() && currentCommand < (int)commandHistory.size() - 1) {
            currentCommand++;
            line = commandHistory[commandHistory.size() - 1 - currentCommand];
        }
    } else if (key == KEY_DOWN) {
        if (currentCommand > 0) {
            currentCommand--;
            line = commandHistory[commandHistory.size() - 1 - currentCommand];
        } else if (currentCommand == 0) {
            currentCommand = -1;
            line.clear();
        }
    }

    // Update the console
    move(LINES-1, 0);
    clrtoeol();
    printw(prompt.c_str());
    printw(line.c_str());
    move(LINES-1, line.length() + prompt.size());  // Move the cursor to the correct position
}

void AdminConsole::cmdReport(const std::string& msg, int colorPair) { // Same functionality as printLog, just different window
    wattron(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wprintw(commandFeedbackWindow, "\n%s", msg.c_str());
    wattroff(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wrefresh(commandFeedbackWindow);
    wmove(commandWindow, 0, line.size() + prompt.size());
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
    wmove(commandWindow, 0, line.size() + prompt.size());
    wrefresh(commandWindow);
}

void AdminConsole::processLine(const std::string& line) {
    if (line.empty()) {
        return;
    }
    if (!isRunning) {
        exit(0);
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

    if (commands.empty())  return;

    int cmdSize = commands.size();

    if (commands[0] == "stop") {
        double value;
        if (cmdSize == 2) {
            if (Config::TryPassDouble(commands[1], value)) {
                stop(value);
            } else {
                cmdReport("Invalid argument for 'stop' command. Argument should be type double.", 4);
            }
        } else if (cmdSize == 1) {
            cmdReport("'stop' command requires a time argument.", 4);
        } else {
            cmdReport("Too many arguments for 'stop' command.", 4);
        }
        

    } else if (commands[0] == "config") {
        if (cmdSize == 1) {
            cmdReport("'config' command requires parameters", 4);
            return;
        }

        if(cmdSize > 3){
            cmdReport("Too many arguments for config command", 4);
            return;
        }
        
        auto it = std::find(secParam[1].begin(), secParam[1].end(), commands[1]);
        if (it == secParam[1].end()) {
            cmdReport("Unknown config parameter: " + commands[1], 4);
            return;
        }
        int index = std::distance(secParam[1].begin(), it);

        if (cmdSize == 2) {
            cmdReport("Setting '" + commands[1] + "' is set to: '" + Config::AccessConfigPointer(index) + '\'', 2);
            return;
        }

        // Size is 3
        if (index <= 2 || index == 9) {
            Config::SetConfigStringValue(index, commands[2]);
            cmdReport("Setting '" + commands[1] + "' is set to: '" + commands[2] + '\'', 2);
            return;
        }
        if (index == 3) {
            if (Config::IsValidIPv4(commands[2])) {
                Config::SetConfigStringValue(index, commands[2]);
                cmdReport("Setting '" + commands[1] + "' is set to: '" + commands[2] + '\'', 2);
                return;
            }
            cmdReport("Invalid IPv4 address", 4);
            return;
        }
        int value;
        if (!Config::TryPassInt(commands[2], value)) {
            cmdReport("Invalid argument for config command. Argument should be type int", 4);
            return;
        }
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
            Config::SetConfigIntValue(index, value);
            cmdReport("Setting '" + commands[1] + "' is set to: '" + std::to_string(value) + '\'', 2);
        }
    } else {
        cmdReport("Unknown command: " + line, 4);
    }
    
}

void AdminConsole::stop(double waitTime) {
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

        cmdReport("Server stopped.", 1);
        exit(0);
    }).detach();
}
