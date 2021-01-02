
// Written by Wouter Ensink

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>


// a simplified version of the juce::AudioProcessor, since we don't need all its functionality,
// but a juce::AudioSource doesn't have midi argument in process function...
class AudioProcessorBase
{
public:
    AudioProcessorBase() = default;
    virtual ~AudioProcessorBase() = default;

    virtual void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {}

    virtual void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) {}

    virtual void releaseResources() {}
};