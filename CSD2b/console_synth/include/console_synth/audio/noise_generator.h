
#pragma once

#include "audio_processor_base.h"

class NoiseGenerator : public AudioProcessorBase
{
public:
    NoiseGenerator() = default;
    ~NoiseGenerator() override = default;

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    void prepareToPlay (double sampleRate, int samplesPerBlockExpected) override {}

    void releaseResources() override {}

private:
    juce::Random random = {};
};
