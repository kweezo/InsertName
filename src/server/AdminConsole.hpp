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

#include "Log.hpp"
#include "Server.hpp"
#include "Config.hpp"


class AdminConsole {
public:
    static void init();

    static std::string readLine(const std::string& prompt);
    static void processLine(const std::string& line);

    static Log* log;

    static void printLog(const std::string& msg, int colorPair = 0);

private:
    static pqxx::connection conn;

    static std::vector<std::string> commands;
    static WINDOW* logWindow;
    static WINDOW* commandWindow;

    static std::deque<std::string> commandHistory;
    static int currentCommand;
    static char line[256];

    static void initWindows();
    static void addCommands();
    static void processKey(int key, const std::string& prompt);

    static void cmdStop();

};