#pragma once

#ifdef _WIN32
    #undef MOUSE_MOVED
    #include <curses.h>
#else
    #include <ncurses.h>
#endif

#include <string>
#include <vector>
#include <deque>


class AdminConsole {
public:
    static void init();
    static std::string readLine();
    static void processLine(const std::string& line);
    static void printLog(const std::string& msg, int colorPair = 0);

private:
    static std::vector<std::string> commands;
    static WINDOW* logWindow;
    static WINDOW* commandWindow;
    static WINDOW* separatorWindow;

    static std::deque<std::string> commandHistory;
    static int currentCommand;
    static std::string prompt;
    static char line[256];
    static int commandWindowHeight;

    static void loadVariables();
    static void initWindows();
    static void addCommands();
    static void processKey(int key, const std::string& prompt);

    static void cmdStop();

};