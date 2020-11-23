

#pragma once

#include "audio_processor_base.h"
#include <juce_audio_processors/juce_audio_processors.h>


class Reverb : public AudioProcessorBase
{
public:
    Reverb()
    {
        auto params = juce::Reverb::Parameters {};
        reverb.setParameters (params);
    }

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        reverb.setSampleRate (sampleRate);
        reverb.reset();
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        auto numSamples = buffer.getNumSamples();
        auto* leftChannel = buffer.getWritePointer (0);
        auto* rightChannel = buffer.getWritePointer (1);
        reverb.processStereo (leftChannel, rightChannel, numSamples);
    }

    void releaseResources() override
    {
        reverb.reset();
    }

private:
    juce::Reverb reverb;
};