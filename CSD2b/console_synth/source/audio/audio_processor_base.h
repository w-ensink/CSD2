
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>


// a simplified version of the juce::AudioProcessor, since we don't need all its functionality,
// but a juce::AudioSource doesn't have midi argument in process function...
class AudioProcessorBase : public juce::AudioProcessor
{
public:
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override {}

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override {}

    void releaseResources() override {}

    bool acceptsMidi() const override { return true; }

    bool producesMidi() const override { return false; }

private:
    // some boiler plate...
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    const juce::String getName() const override { return "internal processor"; }
    double getTailLengthSeconds() const override { return 0; }

    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return "internal processor"; }
    void changeProgramName (int index, const juce::String& newName) override {}
    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}
};