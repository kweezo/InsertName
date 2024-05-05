#include <curses.h>
#include <string>
#include <vector>
#include <cstdlib> // for free


class AdminConsole {
public:
    AdminConsole();
    std::string readLine(const std::string& prompt);
    void processLine(const std::string& line);

private:
    static AdminConsole* currentInstance;
    std::vector<std::string> commands;

    void addCommands();

    void cmdStop();
};