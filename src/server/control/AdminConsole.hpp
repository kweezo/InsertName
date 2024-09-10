#pragma once

#include "Log.hpp"
#include "common/TypeUtils.hpp"
#include "AdvancedSettingsManager.hpp"

#ifdef _WIN32
    #undef MOUSE_MOVED
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#include <array> 
#include <deque>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>

#define COMMAND_COUNT 3


class AdminConsole {
public:
    static void Init();
    static std::string ReadLine();
    static void ProcessLine(const std::string& line);
    static void PrintLog(const std::string& msg, int colorPair = 0);

    static std::atomic<bool> isShuttingDown;
    static std::atomic<bool> isRunning;

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
    static int commandWindowHeight;

    static std::string line;
    static std::string clipboard;
    static size_t cursorPos;
    static size_t selectionStart;
    static size_t selectionEnd;
    static size_t selectionStartOrdered;
    static size_t selectionEndOrdered;

    static std::mutex logMutex;

    static void InitVariables();
    static void InitWindows();
    static void InitColors();
    static void AddCommands();
    
    static void ProcessKey(int key);
    static int FilterKey(int key);
    static void CmdReport(const std::string& msg, int colorPair = 1);

    static void Stop(double waitTime);
};
