
// Written by Wouter Ensink

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

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
    bool handleCommand (std::string_view command);
    [[nodiscard]] std::string getCurrentFeedback() const;

private:
    Engine& engine;
    std::string feedback { "type 'help' to see what's possible" };
    std::vector<std::unique_ptr<CommandHandler>> commandHandlers {};
};


// =================================================================================================

bool isQuitCommand (std::string_view s) noexcept;

std::string fetchUserInput (const std::string& message);