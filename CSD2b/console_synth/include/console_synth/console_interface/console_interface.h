
// Written by Wouter Ensink

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Engine;

// =================================================================================================

struct CommandHandler
{
    virtual ~CommandHandler() = default;
    virtual bool canHandleCommand (std::string_view command) noexcept = 0;
    virtual std::string handleCommand (Engine& engine, std::string_view command) = 0;
    [[nodiscard]] virtual std::string_view getHelpString() const noexcept = 0;
};

// =================================================================================================

struct ConsoleInterface
{
    explicit ConsoleInterface (Engine& engineToControl);
    void handleCommand (std::string_view command);
    void showHelp();
    void run();
    void addCommandHandler (std::unique_ptr<CommandHandler> handler);

private:
    Engine& engine;
    std::string feedback { "type 'help' to see what's possible" };
    std::vector<std::unique_ptr<CommandHandler>> commandHandlers {};
    bool keepRunning = true;

    static constexpr auto header = std::string_view {
        "░██╗░░░░░░░██╗░█████╗░██╗░░░██╗████████╗███████╗██████╗░\n"
        "░██║░░██╗░░██║██╔══██╗██║░░░██║╚══██╔══╝██╔════╝██╔══██╗\n"
        "░╚██╗████╗██╔╝██║░░██║██║░░░██║░░░██║░░░█████╗░░██████╔╝\n"
        "░░████╔═████║░██║░░██║██║░░░██║░░░██║░░░██╔══╝░░██╔══██╗\n"
        "░░╚██╔╝░╚██╔╝░╚█████╔╝╚██████╔╝░░░██║░░░███████╗██║░░██║\n"
        "░░░╚═╝░░░╚═╝░░░╚════╝░░╚═════╝░░░░╚═╝░░░╚══════╝╚═╝░░╚═╝"
    };

    static std::string fetchUserInput (std::string_view message);
    static void clearScreen();

    void printHeader();

};

// =================================================================================================