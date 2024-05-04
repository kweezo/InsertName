#include <linenoise.h>
#include <string>
#include <vector>
#include <cstdlib> // for free

class AdminConsole {
public:
    AdminConsole();
    void addCommand(const std::string& command);
    std::string readLine(const std::string& prompt);

private:
    static AdminConsole* currentInstance;
    std::vector<std::string> commands;

    static void completionCallback(const char* editBuffer, linenoiseCompletions* lc);
};