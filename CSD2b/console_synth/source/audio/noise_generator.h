
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class NoiseGenerator : public juce::AudioSource
{
public:
    NoiseGenerator() = default;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override {}
    void releaseResources() override {}

private:
    juce::Random random = {};
};
