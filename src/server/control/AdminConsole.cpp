#include "AdminConsole.hpp"

#include "ServiceLink.hpp"

bool AdminConsole::isShuttingDown;
std::mutex AdminConsole::isShuttingDownMutex;
bool AdminConsole::isRunning;
std::mutex AdminConsole::isRunningMutex;
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
std::mutex AdminConsole::logMutex;


void AdminConsole::Init() {
    InitVariables();
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    noecho();
    InitColors();
    AddCommands();
    InitWindows();
}

void AdminConsole::InitVariables() {
    commandWindowHeight = AdvancedSettingsManager::GetSettings().commandWindowHeight;
    prompt = AdvancedSettingsManager::GetSettings().commandPrompt;

    {
        std::lock_guard<std::mutex> lock(isShuttingDownMutex);
        isShuttingDown = false;
    }
    {
        std::lock_guard<std::mutex> lock(isRunningMutex);
        isRunning = true;
    }
    currentCommand = -1;
    selectionStart = std::string::npos;
    selectionEnd = std::string::npos;
}

void AdminConsole::InitColors() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
}

void AdminConsole::InitWindows() {
    logWindow = derwin(stdscr, LINES-commandWindowHeight-1, COLS, 0, 0); // Create a new window. Parameters: parent window, number of lines, number of columns, y position, x position
    scrollok(logWindow, TRUE); // Enable scrolling for the log window

    separatorWindow = derwin(stdscr, 1, COLS, LINES-commandWindowHeight-1, 0);

    commandFeedbackWindow = derwin(stdscr, commandWindowHeight-1, COLS, LINES-commandWindowHeight, 0);
    scrollok(commandFeedbackWindow, TRUE);

    commandWindow = derwin(stdscr, 1, COLS, LINES-1, 0);
    mvwhline(separatorWindow, 0, 0, ACS_HLINE, COLS); // Draw a horizontal line
    wrefresh(separatorWindow); // Refresh the separator window

}

void AdminConsole::AddCommands() {
    commands = {"stop", "setting", "save"};

    secParam[0] = {"0"};
    secParam[1] = {
        "serviceid",
        "controlderviceport",

        "loglevel",
        "maxlogbuffersize",

        "dbname",
        "dbuser",
        "dbpassword",
        "dbhostaddr",
        "dbport",

        "commandprompt",
        "commandwindowheight"
    };
    secParam[2] = {"all", "setting", "logs"};
}

bool AdminConsole::IsRunning() {
    std::lock_guard<std::mutex> lock(isRunningMutex);
    return isRunning;
}

bool AdminConsole::IsShuttingDown() {
    std::lock_guard<std::mutex> lock(isShuttingDownMutex);
    return isShuttingDown;
}

std::string AdminConsole::ReadLine() {
    move(LINES-1, 0);
    clrtoeol();
    printw(prompt.c_str());
    wrefresh(commandWindow);

    int ch;
    line = "";
    cursorPos = 0;
    
    while ((ch = getch()) != '\n' && ch != '\r') {
        ProcessKey(ch);

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

int AdminConsole::FilterKey(int key) {
    switch (key) {
        case 8:
        case 127:
            return KEY_BACKSPACE;
    }

    // For ANSI escape sequences

    int sequenceLength = 0;

    if (cursorPos < 3) {
        return key;
    }
    std::string sequence = line.substr(cursorPos - 3, 3) + (char)key;

    if (sequence == "27[D") {
        sequenceLength = 4;
        key = 443; // Ctrl+KEY_LEFT
    } else if (sequence == "27[C") {
        sequenceLength = 4;
        key = 444; // Ctrl+KEY_RIGHT
    }

    if (cursorPos < 4) {
        return key;
    }
    sequence = line.substr(cursorPos - 4, 4) + (char)key;

    if (sequence == "27[1~") {
        sequenceLength = 5;
        key = KEY_HOME;
    } else if (sequence == "27[4~") {
        sequenceLength = 5;
        key = KEY_END;
    }

    if (sequenceLength > 0) {
        line.erase(cursorPos - sequenceLength + 1, sequenceLength - 1);
        cursorPos -= sequenceLength - 1;
    }

    return key;
}

void AdminConsole::ProcessKey(int key) {
    key = FilterKey(key);

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
            if (selectionStart != std::string::npos) {
                cursorPos = selectionStart;
                selectionStart = std::string::npos;
            } else if (cursorPos > 0) {
                cursorPos--;
            }
            selectionStart = std::string::npos;
            break;

        case KEY_RIGHT:
            if (selectionStart != std::string::npos) {
                cursorPos = selectionEndOrdered;
                selectionStart = std::string::npos;
            } else if (cursorPos < line.length()) {
                cursorPos++;
            }
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

void AdminConsole::CmdReport(const std::string& msg, int colorPair) { // Same functionality as printLog, just different window
    wattron(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wprintw(commandFeedbackWindow, "\n%s", msg.c_str());
    wattroff(commandFeedbackWindow, COLOR_PAIR(colorPair));
    wrefresh(commandFeedbackWindow);
    wmove(commandWindow, 0, line.size() + prompt.size());
    wrefresh(commandWindow);
}

void AdminConsole::PrintLog(const std::string& msg, int colorPair) {
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

void AdminConsole::ProcessLine(const std::string& line) {
    bool isRunning_;
    {
        std::lock_guard<std::mutex> lock(isRunningMutex);
        isRunning_ = isRunning;
    }
    if (line.empty() || !isRunning_) {
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

    if (commands.empty())  return;

    int cmdSize = commands.size();

    if (commands[0] == "stop") {
        double value;
        if (cmdSize == 2) {
            if (AdvancedSettingsManager::TryPassDouble(commands[1], value)) {
                Stop(value);
            } else {
                CmdReport("Invalid argument for 'stop' command. Argument should be type double.", 4);
            }
        } else if (cmdSize == 1) {
            CmdReport("'stop' command requires a time argument.", 4);
        } else {
            CmdReport("Too many arguments for 'stop' command.", 4);
        }
        

    } else if (commands[0] == "setting") {
        if (cmdSize == 1) {
            CmdReport("'setting' command requires parameters", 4);
            return;
        }

        if(cmdSize > 3){
            CmdReport("Too many arguments for config command", 4);
            return;
        }
        
        auto it = std::find(secParam[1].begin(), secParam[1].end(), commands[1]);
        if (it == secParam[1].end()) {
            CmdReport("Unknown config parameter: " + commands[1], 4);
            return;
        }
        int index = std::distance(secParam[1].begin(), it);

        if (cmdSize == 2) {
            CmdReport("Setting '" + commands[1] + "' is set to: '" + AdvancedSettingsManager::GetSetting(index) + '\'', 2);
            return;
        }

        // Size is 3
        if (index >= 4 && index <= 6) {
            AdvancedSettingsManager::SetSetting(index, commands[2]);
            CmdReport("Setting '" + commands[1] + "' is set to: '" + commands[2] + '\'', 2);
            return;
        }
        if (index == 7) {
            if (AdvancedSettingsManager::IsValidIPv4(commands[2])) {
                AdvancedSettingsManager::SetSetting(index, commands[2]);
                CmdReport("Setting '" + commands[1] + "' is set to: '" + commands[2] + '\'', 2);
                return;
            }
            CmdReport("Invalid IPv4 address", 4);
            return;
        }

        int value;
        if (!AdvancedSettingsManager::TryPassInt(commands[2], value)) {
            CmdReport("Invalid argument for config command. Argument should be type int", 4);
            return;
        }
        if ((index == 1 || index == 8) && (value < 1 || value > 65535)) {
            CmdReport("Port must be greater than 0 and smaller than 65536", 4);
        } else if ((index == 0 || index == 3 || index == 10) && value < 0) {
            CmdReport("This value must be greater or equal to 0", 4);
        } else if (index == 2 && (value < 0 || value > 4)) {
            CmdReport("Log level must be between 0 and 4", 4);
        } else {
            AdvancedSettingsManager::SetSetting(index, value);
            CmdReport("Setting '" + commands[1] + "' is set to: '" + std::to_string(value) + '\'', 2);
        }
    } else if (commands[0] == "save") {
        if (cmdSize == 1) {
            CmdReport("Saving all...", 2);
            AdvancedSettingsManager::SaveSettings();
            Log::SendLogsToDatabase();
            return;
        }

        if (cmdSize > 2) {
            CmdReport("Too many arguments for save command", 4);
            return;
        }

        auto it = std::find(secParam[2].begin(), secParam[2].end(), commands[1]);
        if (it == secParam[2].end()) {
            CmdReport("Unknown parameter: " + commands[1], 4);
            return;
        }
        int index = std::distance(secParam[2].begin(), it);

        if (index == 0) {
            CmdReport("Saving all...", 2);
            AdvancedSettingsManager::SaveSettings();
            Log::SendLogsToDatabase();
            return;
        }
        if (index == 1) {
            CmdReport("Saving settings...", 2);
            AdvancedSettingsManager::SaveSettings();
            return;
        }
        if (index == 2) {
            CmdReport("Saving logs...", 2);
            Log::SendLogsToDatabase();
            return;
        }
    } else {
        CmdReport("Unknown command: '" + line + '\'', 4);
    }
    
}

void AdminConsole::Stop(double waitTime) {
    CmdReport("Stopping server in " + std::to_string(waitTime) + " minutes...", 2);
    {
        std::lock_guard<std::mutex> lock(isShuttingDownMutex);
        isShuttingDown = true;
    }
    ServiceLink::NotifyConnection();

    std::thread([waitTime]() {
        int waitTimeInSeconds = static_cast<int>(waitTime * 60);
        std::this_thread::sleep_for(std::chrono::seconds(waitTimeInSeconds));

        CmdReport("Stopping server...", 2);
        Log::Print("Server stopped by admin command", 1);
        {
            std::lock_guard<std::mutex> lock(isRunningMutex);
            isRunning = false;
        }

        AdvancedSettingsManager::SaveSettings();
        Log::Destroy();

        CmdReport("Server stopped. Press ENTER to exit.", 1);
    }).detach();
}
