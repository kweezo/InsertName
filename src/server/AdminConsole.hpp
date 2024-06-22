#pragma once

#ifdef _WIN32
    #undef MOUSE_MOVED
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

#include <string>
#include <vector>
#include <array> 
#include <deque>

#define COMMAND_COUNT 2

class AdminConsole {
public:
    static void init();
    static std::string readLine();
    static void processLine(const std::string& line);
    static void printLog(const std::string& msg, int colorPair = 0);

    static bool isRunning;

private:
    static std::array<std::string, COMMAND_COUNT> commands;
    static std::array<std::vector<std::string>, COMMAND_COUNT> secParam;

    static WINDOW* logWindow;
    static WINDOW* separatorWindow;
    static WINDOW* commandWindow;
    static WINDOW* commandFeedbackWindow;

    static std::deque<std::string> commandHistory;
    static int currentCommand;
    static std::string prompt;
    static std::string line;
    static int commandWindowHeight;

    static void loadVariables();
    static void initWindows();
    static void initColors();
    static void addCommands();
    
    static void processKey(int key, const std::string& prompt);
    static void cmdReport(const std::string& msg, int colorPair = 1);

    static void stop(double waitTime);
};