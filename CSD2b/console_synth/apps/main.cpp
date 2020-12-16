
// Written by Wouter Ensink

#include <console_synth/audio/audio_engine.h>
#include <console_synth/audio/reverb.h>
#include <console_synth/audio/saw_synthesizer.h>
#include <console_synth/console_interface/console_interface.h>
#include <console_synth/format.h>
#include <console_synth/utility.h>

#include <console_synth/sequencer/sequencer.h>
// ==============================================================


int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;


    auto root = juce::ValueTree { "root" };

    auto engine = Engine { root };

    auto handler = ChangeTempoCommand {};

    for (;;)
    {
        auto input = fetchUserInput (" -> ");

        if (isQuitCommand (input))
            break;

        if (handler.canHandleCommand (input))
        {
            auto feedback = handler.handleCommand (engine, input);

            fmt::print ("handled command {} with answer: {}\n", input, feedback);
        }

        if (auto d = attemptSetMidiCommand (input))
            fmt::print ("setting midi device to {}\n", *d);
    }


    return 0;
}
