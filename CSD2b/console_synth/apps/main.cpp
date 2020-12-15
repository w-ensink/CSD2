
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

    /*
    auto rootAudioSource = SawSynthesizer (4);
    auto reverb = std::make_unique<Reverb>();

    auto processorChain = ProcessorChain (rootAudioSource);
    processorChain.addEffectToChain (std::move (reverb));

    auto engine = AudioEngine (processorChain);

    fmt::print ("press enter to exit... \n");
    std::cin.get();
     */

    auto synth = SawSynthesizer { 4 };
    auto sequencer = Sequencer { synth };
    sequencer.setTempoBpm(100);

    auto engine = AudioEngine { sequencer };

    for (;;)
    {
        auto input = fetchUserInput (" -> ");

        if (isQuitCommand (input))
            break;

        if (auto d = attemptSetMidiCommand (input))
            fmt::print ("setting midi device to {}\n", *d);
    }


    return 0;
}
