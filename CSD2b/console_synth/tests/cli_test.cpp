
// Written by Wouter Ensink

/*
#include "../source/console_interface.h"
#include <catch2/catch_all.hpp>

// the cli works by providing commands with callbacks
// the command handler is separate from the rest

TEST_CASE ("CliTest", "Setup")
{
    auto dispatcher = CommandDispatcher();
    dispatcher.addCommandHandler ("freq", [] (juce::String cmd) {
        REQUIRE (cmd == "freq");
        return cmd;
    });

    dispatcher.dispatch ("freq");
}
 */