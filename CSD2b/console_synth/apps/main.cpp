
// Written by Wouter Ensink

#include <console_synth/audio/audio_engine.h>
#include <console_synth/console_interface/console_interface.h>
#include <console_synth/utility.h>
#include <fmt/color.h>

int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;

    auto engine = Engine {};
    auto consoleInterface = ConsoleInterface { engine };

    for (;;)
    {
        auto input = fetchUserInput (" --> ");

        if (isQuitCommand (input))
            break;

        if (consoleInterface.handleCommand (input))
            fmt::print (fg (fmt::color::light_sky_blue), "{}\n", consoleInterface.getCurrentFeedback());
    }

    return 0;
}
