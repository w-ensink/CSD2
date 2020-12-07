
// Written by Wouter Ensink

#include <console_synth/audio/audio_engine.h>
#include <console_synth/audio/reverb.h>
#include <console_synth/audio/saw_synthesizer.h>
#include <console_synth/format.h>
#include <console_synth/console_interface/console_interface.h>
#include <console_synth/utility.h>

// ==============================================================


class ProcessorChain : public AudioProcessorBase
{
public:
    explicit ProcessorChain (AudioProcessorBase& rootInstrument) : rootInstrument (rootInstrument) {}

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        rootInstrument.prepareToPlay (sampleRate, maximumExpectedSamplesPerBlock);

        for (auto& e : effects)
            e->prepareToPlay (sampleRate, maximumExpectedSamplesPerBlock);
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        rootInstrument.processBlock (buffer, midiMessages);

        for (auto& e : effects)
            e->processBlock (buffer, midiMessages);
    }

    void releaseResources() override
    {
        rootInstrument.releaseResources();

        for (auto& e : effects)
            e->releaseResources();
    }

    template <typename EffectType>
    void addEffectToChain (std::unique_ptr<EffectType> effect)
    {
        effects.push_back (std::move (effect));
    }

private:
    AudioProcessorBase& rootInstrument;
    std::vector<std::unique_ptr<AudioProcessorBase>> effects;
};





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
