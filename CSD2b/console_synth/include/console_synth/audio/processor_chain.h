
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/audio_processor_base.h>


class ProcessorChain : public AudioProcessorBase
{
public:
    explicit ProcessorChain (AudioProcessorBase& rootInstrument) : rootInstrument (rootInstrument) {}

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        rootInstrument.prepareToPlay (sampleRate, maximumExpectedSamplesPerBlock);

        std::for_each (effects.begin(), effects.end(), [&] (auto& e) {
            e->prepareToPlay (sampleRate, maximumExpectedSamplesPerBlock);
        });
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        rootInstrument.processBlock (buffer, midiMessages);

        std::for_each (effects.begin(), effects.end(), [&] (auto& e) {
            e->processBlock (buffer, midiMessages);
        });
    }

    void releaseResources() override
    {
        rootInstrument.releaseResources();

        std::for_each (effects.begin(), effects.end(), [] (auto& e) {
            e->releaseResources();
        });
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
