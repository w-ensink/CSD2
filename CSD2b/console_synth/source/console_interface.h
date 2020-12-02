
// Written by Wouter Ensink

#pragma once

/*
#include "format.h"

struct CommandHandlerBase
{
    CommandHandlerBase() = default;
    virtual ~CommandHandlerBase() = default;

    virtual juce::String handle (const juce::String& command) = 0;
    virtual bool matchesCommand (const juce::String& command) = 0;
};


template <typename FunctorType>
struct CommandHandler : CommandHandlerBase
{
    CommandHandler (const juce::String& cmd, FunctorType handler) : command (cmd), handler (handler) {}
    ~CommandHandler() override = default;

    juce::String handle (const juce::String& cmd) override
    {
        return handler (cmd);
    }

    bool matchesCommand (const juce::String& cmd) override
    {
        return command == cmd;
    }

    juce::String command;
    FunctorType handler;
};


template <typename FunctorType>
auto makeCommandHandler (juce::String cmd, FunctorType handler)
{
    return std::make_unique<CommandHandler<FunctorType>> (cmd, handler);
}


class CommandDispatcher
{
public:
    void addCommandHandler (std::unique_ptr<CommandHandlerBase> handler)
    {
        handlers.push_back (std::move (handler));
    }

    template <typename FunctorType>
    void addCommandHandler (const juce::String& cmd, FunctorType handler)
    {
        handlers.push_back (makeCommandHandler (cmd, handler));
    }

    juce::String dispatch (const juce::String& cmd)
    {
        for (auto& h : handlers)
        {
            if (h->matchesCommand (cmd))
                return h->handle (cmd);
        }

        throw std::runtime_error ("command not handled");
    }

private:
    template <typename T>
    using OwnedVector = std::vector<std::unique_ptr<T>>;

    OwnedVector<CommandHandlerBase> handlers;
};
 */