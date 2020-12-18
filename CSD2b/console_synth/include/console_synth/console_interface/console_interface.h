
#pragma once

#include <console_synth/audio/audio_engine.h>
#include <console_synth/format.h>
#include <ctre.hpp>
#include <optional>


std::optional<std::string_view> attemptSetMidiCommand (std::string_view s) noexcept
{
    static constexpr auto pattern = ctll::fixed_string { "^set\\smidi\\s([a-zA-Z0-9]+)$" };

    if (auto m = ctre::match<pattern> (s))
        return m.get<1>().to_view();

    return std::nullopt;
}

// =================================================================================================

struct CommandHandler
{
    virtual ~CommandHandler() = default;
    virtual bool canHandleCommand (std::string_view command) noexcept = 0;
    virtual std::string handleCommand (Engine& engine, std::string_view command) = 0;
    [[nodiscard]] virtual std::string_view getHelpString() const noexcept = 0;
};

// =================================================================================================

struct ChangeTempoCommandHandler : public CommandHandler
{
    ~ChangeTempoCommandHandler() override = default;

    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto num = std::stod (ctre::match<pattern> (command).get<1>().to_string());
        engine.getValueTreeState().getChildWithName (IDs::sequencer).setProperty (IDs::tempo, num, nullptr);
        return fmt::format ("set tempo to {}", num);
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "tempo <tempo_bpm> (sets sequencer tempo)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^tempo\\s([0-9]+)$" };
};

// =================================================================================================

struct ListAudioDevicesCommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto deviceNames = engine.getAvailableAudioDevices();

        auto nameList = std::string { "\n" };

        for (auto& name : deviceNames)
            nameList += fmt::format (" - {}\n", name);

        return nameList;
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "ls audio devices (gives list of all available audio devices)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^ls\\saudio\\sdevices$" };
};

// =================================================================================================

struct ListMidiDevicesCommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto deviceNames = Engine::getAvailableMidiDevices();

        auto nameList = std::string { "\n" };

        for (auto& name : deviceNames)
            nameList += fmt::format (" - {}\n", name);

        return nameList;
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "ls midi devices (gives a list of available midi input devices)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^ls\\smidi\\sdevices$" };
};

// =================================================================================================


struct OpenMidiInputDevice_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto name = ctre::match<pattern> (command).get<1>().to_string();

        if (engine.getSequencer().openMidiInputDevice (name))
            return fmt::format ("Successfully opened midi input device '{}'", name);

        return fmt::format ("Failed to open '{}'", name);
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "open midi <midi_input_name> (attemts to open midi input device)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^open\\smidi\\s([a-zA-Z0-9\\s]+)$" };
};


// =================================================================================================

struct ConsoleInterface
{
    explicit ConsoleInterface (Engine& engineToControl) : engine { engineToControl }
    {
        commandHandlers.push_back (std::make_unique<ChangeTempoCommandHandler>());
        commandHandlers.push_back (std::make_unique<ListAudioDevicesCommandHandler>());
        commandHandlers.push_back (std::make_unique<ListMidiDevicesCommandHandler>());
        commandHandlers.push_back (std::make_unique<OpenMidiInputDevice_CommandHandler>());
    }

    bool handleCommand (std::string_view command)
    {
        for (auto&& handler : commandHandlers)
        {
            if (handler->canHandleCommand (command))
            {
                feedback = handler->handleCommand (engine, command);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string getCurrentFeedback() const
    {
        return feedback;
    }

private:
    Engine& engine;
    std::string feedback { "type 'help' to see what's possible" };

    std::vector<std::unique_ptr<CommandHandler>> commandHandlers {};
};

// =================================================================================================

auto isQuitCommand (std::string_view s) noexcept
{
    static constexpr auto pattern = ctll::fixed_string { "^(quit|exit)$" };

    if (ctre::match<pattern> (s))
        return true;

    return false;
}

auto fetchUserInput (const std::string& message)
{
    fmt::print ("{}", message);
    auto targetString = std::string {};
    std::getline (std::cin, targetString);
    return targetString;
}