
// Written by Wouter Ensink

#include <console_synth/console_interface/console_interface.h>
#include <console_synth/engine.h>
#include <console_synth/sequencer/melody_generator.h>
#include <console_synth/utility/format.h>
#include <ctre.hpp>
#include <thread>

// =================================================================================================

struct ChangeTempo_CommandHandler : public CommandHandler
{
    ~ChangeTempo_CommandHandler() override = default;

    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto num = std::stod (ctre::match<pattern> (command).get<1>().to_string());
        engine.getValueTreeState().getChildWithName (IDs::sequencer).setProperty (IDs::tempo, num, engine.getUndoManager());
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

struct ListAudioDevices_CommandHandler : public CommandHandler
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

struct ListMidiDevices_CommandHandler : public CommandHandler
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
    static constexpr auto pattern = ctll::fixed_string { R"(^open\smidi\s([a-zA-Z0-9\s]+)$)" };
};


// =================================================================================================

struct StopPlayback_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        engine.getSequencer().stopPlayback();
        return "stopped playback";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "stop (stops playback)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^stop$" };
};

// =================================================================================================

struct StartPlayback_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        engine.getSequencer().startPlayback();
        return "started playback";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "start (start playback)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^start$" };
};

// =================================================================================================

struct AddNote_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto note = juce::ValueTree { IDs::note };

        if (auto m = ctre::match<pattern> (command))
        {
            auto noteNumber = std::stoi (m.get<1>().to_string());
            auto start = std::stoi (m.get<2>().to_string());

            auto length = 48;
            auto velocity = 127;

            note.setProperty (IDs::midiNoteNumber, noteNumber, nullptr);
            note.setProperty (IDs::startTimeTicks, start, nullptr);
            note.setProperty (IDs::lengthTicks, length, nullptr);
            note.setProperty (IDs::velocity, velocity, nullptr);

            engine.getValueTreeState()
                .getChildWithName (IDs::sequencer)
                .getChildWithName (IDs::track)
                .getChildWithName (IDs::melody)
                .addChild (note, -1, engine.getUndoManager());

            return fmt::format ("added note {} at {}", noteNumber, start);
        }

        return "failed to add note";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "add note <number> <position_ticks>";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { R"(^add\snote\s([0-9]+)\s([0-9]+)$)" };
};

// =================================================================================================

struct ListNotes_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto melody = engine.getValueTreeState()
                          .getChildWithName (IDs::sequencer)
                          .getChildWithName (IDs::track)
                          .getChildWithName (IDs::melody);

        auto answer = std::string {};

        for (auto&& note : melody)
        {
            auto midiNum = (int) note.getProperty (IDs::midiNoteNumber);
            auto start = (int) note.getProperty (IDs::startTimeTicks);
            auto length = (int) note.getProperty (IDs::lengthTicks);
            auto velocity = (int) note.getProperty (IDs::velocity);

            answer += fmt::format ("number: {},\tstart: {},\tlength: {},\tvelocity: {}\n", midiNum, start, length, velocity);
        }

        return answer;
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "ls notes (gives a formatted list of all notes in the melody)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^ls\\snotes$" };
};

// =================================================================================================

struct GenerateMelody_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto generator = MelodyGenerator {};
        auto melody = generator.generateMelody (engine.getSequencer().getTimeSignature());

        auto m = engine.getValueTreeState()
                     .getChildWithName (IDs::sequencer)
                     .getChildWithName (IDs::track)
                     .getChildWithName (IDs::melody);

        auto shouldPausePlayback = engine.getSequencer().isPlaying();

        if (shouldPausePlayback)
            engine.getSequencer().stopPlayback();

        // kind of a hack to make all notes currently playing stop... (wait for more than 1 buffer length)
        std::this_thread::sleep_for (std::chrono::milliseconds { 50 });

        m.removeAllChildren (nullptr);
        m.copyPropertiesAndChildrenFrom (melody, nullptr);

        if (shouldPausePlayback)
            engine.getSequencer().startPlayback();

        return "generated melody";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "g (generates melody)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^g$" };
};

// =================================================================================================

struct Undo_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        if (engine.getUndoManager()->undo())
            return "undo successful";

        return "undo failed";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "undo (undoes effect of last command)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^undo$" };
};

// =================================================================================================

struct Redo_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        if (engine.getUndoManager()->redo())
            return "redo successful";

        return "redo failed";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "redo (redoes effect of last undone command)";
    }

private:
    static constexpr auto pattern = ctll::fixed_string { "^redo$" };
};

// =================================================================================================

struct ChangeEnvelope_CommandHandler : public CommandHandler
{
    bool canHandleCommand (std::string_view command) noexcept override
    {
        return ctre::match<pattern> (command);
    }

    std::string handleCommand (Engine& engine, std::string_view command) override
    {
        auto synth = engine.getValueTreeState()
                         .getChildWithName (IDs::sequencer)
                         .getChildWithName (IDs::track)
                         .getChildWithName (IDs::synth);

        auto match = ctre::match<pattern> (command);

        try
        {
            auto a = std::stod (match.get<1>().to_string());
            auto d = std::stod (match.get<2>().to_string());
            auto s = std::stod (match.get<3>().to_string());
            auto r = std::stod (match.get<4>().to_string());

            synth.setProperty (IDs::attack, a, nullptr);
            synth.setProperty (IDs::decay, d, nullptr);
            synth.setProperty (IDs::sustain, s, nullptr);
            synth.setProperty (IDs::release, r, nullptr);
        }
        catch (std::exception& e)
        {
            return fmt::format ("error in parsing envelope command: {}", e.what());
        }

        return "adsr set successfully";
    }

    [[nodiscard]] std::string_view getHelpString() const noexcept override
    {
        return "adsr <a> <d> <s> <r> (sets the envelope)";
    }

    // regex for: "adsr <float> <float> <float> <float>"
    static constexpr auto pattern = ctll::fixed_string { R"(^adsr\s(\d+\.\d+)\s(\d+\.\d+)\s(\d+\.\d+)\s(\d+\.\d+)$)" };
};

// =================================================================================================


ConsoleInterface::ConsoleInterface (Engine& engineToControl) : engine { engineToControl }
{
    addCommandHandler (std::make_unique<StartPlayback_CommandHandler>());
    addCommandHandler (std::make_unique<StopPlayback_CommandHandler>());
    addCommandHandler (std::make_unique<ChangeTempo_CommandHandler>());
    addCommandHandler (std::make_unique<ListAudioDevices_CommandHandler>());
    addCommandHandler (std::make_unique<ListMidiDevices_CommandHandler>());
    addCommandHandler (std::make_unique<OpenMidiInputDevice_CommandHandler>());
    addCommandHandler (std::make_unique<AddNote_CommandHandler>());
    addCommandHandler (std::make_unique<ListNotes_CommandHandler>());
    addCommandHandler (std::make_unique<GenerateMelody_CommandHandler>());
    addCommandHandler (std::make_unique<Undo_CommandHandler>());
    addCommandHandler (std::make_unique<Redo_CommandHandler>());
    addCommandHandler (std::make_unique<ChangeEnvelope_CommandHandler>());
}

void ConsoleInterface::handleCommand (std::string_view command)
{
    for (auto&& handler : commandHandlers)
    {
        if (handler->canHandleCommand (command))
        {
            feedback = handler->handleCommand (engine, command);
            return;
        }
    }

    if (command == "help")
    {
        showHelp();
        return;
    }

    if (command == "quit" || command == "exit")
    {
        keepRunning = false;
        return;
    }

    feedback = fmt::format ("'{}' is not a valid command, enter 'help' to see what's possible", command);
}


void ConsoleInterface::showHelp()
{
    auto help = std::string {};

    for (auto&& handler : commandHandlers)
        help += fmt::format (" - {}\n", handler->getHelpString());


    fmt::print (help);
    fetchUserInput ("Press enter to return... ");
}


void ConsoleInterface::run()
{
    static constexpr auto header = std::string_view {
        "░██╗░░░░░░░██╗░█████╗░██╗░░░██╗████████╗███████╗██████╗░\n"
        "░██║░░██╗░░██║██╔══██╗██║░░░██║╚══██╔══╝██╔════╝██╔══██╗\n"
        "░╚██╗████╗██╔╝██║░░██║██║░░░██║░░░██║░░░█████╗░░██████╔╝\n"
        "░░████╔═████║░██║░░██║██║░░░██║░░░██║░░░██╔══╝░░██╔══██╗\n"
        "░░╚██╔╝░╚██╔╝░╚█████╔╝╚██████╔╝░░░██║░░░███████╗██║░░██║\n"
        "░░░╚═╝░░░╚═╝░░░╚════╝░░╚═════╝░░░░╚═╝░░░╚══════╝╚═╝░░╚═╝"
    };

    while (keepRunning)
    {
        clearScreen();
        fmt::print ("{}\n\n", header);
        fmt::print ("{}\n", feedback);
        auto command = fetchUserInput (" --> ");

        handleCommand (command);
    }
}

std::string ConsoleInterface::fetchUserInput (std::string_view message)
{
    fmt::print ("{}", message);
    auto targetString = std::string {};
    std::getline (std::cin, targetString);
    return targetString;
}


void ConsoleInterface::clearScreen()
{
#if defined(JUCE_MAC) || defined(JUCE_LINUX)
    system ("clear");
#elif defined(JUCE_WINDOWS)
    system ("cls");
#endif
}


void ConsoleInterface::addCommandHandler (std::unique_ptr<CommandHandler> handler)
{
    commandHandlers.push_back (std::move (handler));
}