
#include "audio_callback.h"
#include "../format.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>


AudioCallback::AudioCallback (juce::AudioProcessor& source) : audioSource (source) {};

void AudioCallback::audioDeviceIOCallback (const float** inputChannelData,
                                           int numInputChannels,
                                           float** outputChannelData,
                                           int numOutputChannels,
                                           int numSamples)
{
    auto buffer = juce::AudioBuffer<float> (outputChannelData, numOutputChannels, numSamples);
    auto midiBuffer = juce::MidiBuffer();
    audioSource.processBlock (buffer, midiBuffer);
}


void AudioCallback::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    fmt::print ("{} about to start:\n"
                "\tsample rate: {}\n"
                "\tbuffer size: {}\n"
                "\tbit depth:   {}\n",
                device->getName(),
                device->getCurrentSampleRate(),
                device->getCurrentBufferSizeSamples(),
                device->getCurrentBitDepth());

    audioSource.prepareToPlay (device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
}


void AudioCallback::audioDeviceStopped()
{
    audioSource.releaseResources();
    fmt::print ("Audio Device Stopped\n");
}


void AudioCallback::audioDeviceError (const juce::String& errorMessage)
{
    fmt::print ("Audio Device Error: {}\n", errorMessage);
}