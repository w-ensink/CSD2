
// Written by Wouter Ensink

#include "format.h"
#include <audio/audio_engine.h>
#include <audio/saw_synthesizer.h>
#include <audio/sine_synthesizer.h>

// uses RAII to start and stop the message thread
struct ScopedMessageThread
{
    ScopedMessageThread() { juce::initialiseJuce_GUI(); }
    ~ScopedMessageThread() { juce::shutdownJuce_GUI(); }
};

#define SCOPE_ENABLE_MESSAGE_THREAD \
    ScopedMessageThread smt


// ==============================================================

int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;

    //auto rootProcessor = SineSynthesizer (4);
    auto rootAudioSource = SawSynthesizer (4);
    auto engine = AudioEngine (rootAudioSource);

    fmt::print ("press enter to exit... \n");
    std::cin.get();

    return 0;
}
