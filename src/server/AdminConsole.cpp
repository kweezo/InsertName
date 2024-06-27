#include "AdminConsole.hpp"

#include "Config.hpp"
#include "Server.hpp"
#include "Log.hpp"

#include <algorithm>
#include <thread>
#include <regex>


bool AdminConsole::isRunning;
std::array<std::string, COMMAND_COUNT> AdminConsole::commands;
std::array<std::vector<std::string>, COMMAND_COUNT> AdminConsole::secParam;
WINDOW* AdminConsole::logWindow = nullptr;
WINDOW* AdminConsole::separatorWindow = nullptr;
WINDOW* AdminConsole::commandWindow = nullptr;
WINDOW* AdminConsole::commandFeedbackWindow = nullptr;
std::deque<std::string> AdminConsole::commandHistory;
int AdminConsole::currentCommand;
std::string AdminConsole::prompt;
int AdminConsole::commandWindowHeight;
std::string AdminConsole::line;
std::string AdminConsole::clipboard;
size_t AdminConsole::cursorPos;
size_t AdminConsole::selectionStart;
size_t AdminConsole::selectionEnd;
size_t AdminConsole::selectionStartOrdered;
size_t AdminConsole::selectionEndOrdered;

void AdminConsole::init() {
    initVariables();
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    noecho();
    initColors();
    addCommands();
    initWindows();
}

void AdminConsole::initVariables() {
    commandWindowHeight = Config::commandWindowHeight;
    prompt = Config::commandPrompt;

    isRunning = true;
    currentCommand = -1;
    selectionStart = std::string::npos;
    selectionEnd = std::string::npos;
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
    commands = {"stop", "config", "save"};

    secParam[0] = {"0"};
    secParam[1] = {"dbname", "dbuser", "dbpassword", "dbhostaddr", "dbport", "serverport", "loginattempts", "loglevel", "maxlogbuffersize", "commandprompt", "commandwindowheight"};
    secParam[2] = {"all", "config", "logs"};
}

std::string AdminConsole::readLine() {
    move(LINES-1, 0); // Move the cursor to the command line
    clrtoeol(); // Clear the command line
    printw(prompt.c_str()); // Print the command prefix
    wrefresh(commandWindow);

    int ch;
    line = "";
    cursorPos = 0;
    
    while ((ch = getch()) != '\n' && ch != '\r') {
        processKey(ch);

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

int AdminConsole::filterKey(int key) {
    switch (key) {
        case 8:
        case 127:
            return KEY_BACKSPACE;
    }

    // For ANSI escape sequences

    if (line.length() < 4) {
        return key;
    }

    std::string sequence = line.substr(cursorPos - 4, 4);
    int sequenceLength = 0;

    if (sequence == "27[D") {
        sequenceLength = 4;
        return 443; // Ctrl+KEY_LEFT
    } else if (sequence == "27[C") {
        sequenceLength = 4;
        return 444; // Ctrl+KEY_RIGHT
    }

    sequence = line.substr(cursorPos - 5, 5);

    if (sequence == "27[1~") {
        sequenceLength = 5;
        return KEY_HOME;
    } else if (sequence == "27[4~") {
        sequenceLength = 5;
        return KEY_END;
    }

    if (sequenceLength > 0) {
        line.erase(cursorPos - sequenceLength, sequenceLength);
        cursorPos -= sequenceLength;
    }

    return key;
}

void AdminConsole::processKey(int key) {
    key = filterKey(key);

    switch (key) {
        case 443: // Ctrl+KEY_LEFT
            // Move cursorPos to the start of the previous word
            if (cursorPos > 0) {
                // Skip any spaces before the current position
                while (cursorPos > 0 && line[cursorPos - 1] == ' ') cursorPos--;
                // Move to the start of the word
                while (cursorPos > 0 && line[cursorPos - 1] != ' ') cursorPos--;
            }
            selectionStart = std::string::npos;
            break;

        case 444: // Ctrl+KEY_RIGHT
            // Move cursorPos to the start of the next word
            if (cursorPos < line.length()) {
                // Skip current word
                while (cursorPos < line.length() && line[cursorPos] != ' ') cursorPos++;
                // Skip any spaces after the current word
                while (cursorPos < line.length() && line[cursorPos] == ' ') cursorPos++;
            }
            selectionStart = std::string::npos;
            break;

        case 23: // Ctrl+Backspace
            if (cursorPos > 0) {
                size_t startPos = cursorPos;
                // Skip any spaces before the current position
                while (cursorPos > 0 && line[cursorPos - 1] == ' ') cursorPos--;
                // Move to the start of the word
                while (cursorPos > 0 && line[cursorPos - 1] != ' ') cursorPos--;
                // Erase the word
                line.erase(cursorPos, startPos - cursorPos);
            }
            selectionStart = std::string::npos;
            break;
        
        case 420: // Ctrl+Delete
            if (cursorPos < line.length()) {
                size_t startPos = cursorPos;
                // Skip current word
                while (cursorPos < line.length() && line[cursorPos] != ' ') cursorPos++;
                // Skip any spaces after the current word
                while (cursorPos < line.length() && line[cursorPos] == ' ') cursorPos++;
                // Erase the word
                line.erase(startPos, cursorPos - startPos);
                cursorPos = startPos; // Reset cursor position to the start of deletion
            }
            selectionStart = std::string::npos;
            break;

        case 391: // Shift+Left
            if (selectionStart == std::string::npos) {
                selectionStart = cursorPos;
            }
            if (cursorPos > 0) {
                cursorPos--;
                selectionEnd = cursorPos;
            }
            break;

        case 400: // Shift+Right
            if (selectionStart == std::string::npos) {
                selectionStart = cursorPos;
            }
            if (cursorPos < line.length()) {
                cursorPos++;
                selectionEnd = cursorPos;
            }
            break;

        case 1: // Ctrl+A
            selectionStart = 0;
            selectionEnd = line.length();
            break;

        case 24: // Ctrl+X
            if (selectionStartOrdered != std::string::npos && selectionEndOrdered != std::string::npos) {
                clipboard = line.substr(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered);
                line.erase(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered);
                cursorPos = selectionStartOrdered;
                selectionStart = std::string::npos;
            }
            break;

        case 3: // Ctrl+C
            if (selectionStartOrdered != std::string::npos && selectionEndOrdered != std::string::npos) {
                clipboard = line.substr(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered);
                selectionStart = std::string::npos;
            }
            break;

        case 22: // Ctrl+V
            if (!clipboard.empty()) {
                line.insert(cursorPos, clipboard);
                cursorPos += clipboard.length();
                selectionStart = std::string::npos;
            }
            break;

        case KEY_HOME:
            cursorPos = 0;
            selectionStart = std::string::npos;
            break;

        case KEY_END:
            cursorPos = line.length();
            selectionStart = std::string::npos;
            break;

        case KEY_LEFT:
            if (cursorPos > 0) cursorPos--;
            selectionStart = std::string::npos;
            break;

        case KEY_RIGHT:
            if (cursorPos < line.length()) cursorPos++;
            selectionStart = std::string::npos;
            break;

        case KEY_UP:
            if (!commandHistory.empty() && currentCommand < (int)commandHistory.size() - 1) {
                currentCommand++;
                line = commandHistory[commandHistory.size() - 1 - currentCommand];
            }
            cursorPos = line.size();
            selectionStart = std::string::npos;
            break;

        case KEY_DOWN:
            if (currentCommand > 0) {
                currentCommand--;
                line = commandHistory[commandHistory.size() - 1 - currentCommand];
            } else if (currentCommand == 0) {
                currentCommand = -1;
                line.clear();
            }
            cursorPos = line.size();
            selectionStart = std::string::npos;
            break;

        case '\t': {
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
                    return;
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
                                selectionStart = std::string::npos;
                                break;
                            }
                        }
                    }
                    
                    // Copy the common prefix to the input line
                    line = ((spacePos != std::string::npos && commandIndex != -1) ? commands[commandIndex] + ' ' : "") + commonPrefix;
                }
            }

            cursorPos = line.size();
            }
            selectionStart = std::string::npos;
            break;

        case KEY_BACKSPACE:
            if (selectionStartOrdered != std::string::npos && selectionEndOrdered != std::string::npos) {
                line.erase(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered); // Remove selected text
                cursorPos = selectionStartOrdered;

            } else if (cursorPos > 0) {
                line.erase(cursorPos - 1, 1); // Remove character before the cursor
                cursorPos--;
            }

            selectionStart = std::string::npos;
            break;

        case KEY_DC:
            if (selectionStartOrdered != std::string::npos && selectionEndOrdered != std::string::npos) {
                line.erase(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered); // Remove selected text
                cursorPos = selectionStartOrdered;

            } else if (cursorPos < line.length()) {
                line.erase(cursorPos, 1); // Remove character at the cursor position
            }

            selectionStart = std::string::npos;
            break;
        
        default:
            if (key >= 32 && key <= 126 && line.length() < COLS - prompt.size() - 1) {
                line.insert(cursorPos, 1, key); // Insert character at cursor
                cursorPos++;
            }
            else{line+=std::to_string(key);cursorPos+=std::to_string(key).size();} // For debugging purposes

            selectionStart = std::string::npos;
            break;
    }

    selectionStartOrdered = selectionStart;
    selectionEndOrdered = selectionEnd;
    if (selectionStartOrdered > selectionEndOrdered) {
        std::swap(selectionStartOrdered, selectionEndOrdered);
    }

    // Update the console
    move(LINES-1, 0);
    clrtoeol();
    printw(prompt.c_str());


    if (selectionStartOrdered != std::string::npos && selectionEndOrdered != std::string::npos) {
        std::string beforeSelection = line.substr(0, selectionStartOrdered);
        printw(beforeSelection.c_str());

        attron(A_REVERSE);
        std::string selectedText = line.substr(selectionStartOrdered, selectionEndOrdered - selectionStartOrdered);
        printw(selectedText.c_str());
        attroff(A_REVERSE);

        std::string afterSelection = line.substr(selectionEndOrdered);
        printw(afterSelection.c_str());

    } else {
        printw(line.c_str());
    }

    move(LINES-1, cursorPos + prompt.size());
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
    } else if (commands[0] == "save") {
        if (cmdSize == 1) {
            cmdReport("Saving all...", 2);
            Config::SaveConfig();
            Log::sendLogsToDatabase();
            return;
        }

        if (cmdSize > 2) {
            cmdReport("Too many arguments for save command", 4);
            return;
        }

        auto it = std::find(secParam[2].begin(), secParam[2].end(), commands[1]);
        if (it == secParam[2].end()) {
            cmdReport("Unknown parameter: " + commands[1], 4);
            return;
        }
        int index = std::distance(secParam[2].begin(), it);

        if (index == 0) {
            cmdReport("Saving all...", 2);
            Config::SaveConfig();
            Log::sendLogsToDatabase();
            return;
        }
        if (index == 1) {
            cmdReport("Saving config...", 2);
            Config::SaveConfig();
            return;
        }
        if (index == 2) {
            cmdReport("Saving logs...", 2);
            Log::sendLogsToDatabase();
            return;
        }
    } else {
        cmdReport("Unknown command: '" + line + '\'', 4);
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
