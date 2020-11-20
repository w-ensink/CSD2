
// Written by Wouter Ensink

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>


class AudioCallback : public juce::AudioIODeviceCallback
{
public:
    explicit AudioCallback (juce::AudioSource& source);
    ~AudioCallback() override = default;

    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples) override;

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;

    void audioDeviceStopped() override;

    void audioDeviceError (const juce::String& errorMessage) override;

private:
    juce::AudioSource& audioSource;
};
