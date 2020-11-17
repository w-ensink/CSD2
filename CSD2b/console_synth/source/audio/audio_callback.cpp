
#include "audio_callback.h"
#include "../format.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>


AudioCallback::AudioCallback (juce::AudioSource& source) : audioSource (source) {};

void AudioCallback::audioDeviceIOCallback (const float** inputChannelData,
                                           int numInputChannels,
                                           float** outputChannelData,
                                           int numOutputChannels,
                                           int numSamples)
{
    auto buffer = juce::AudioBuffer<float> (outputChannelData, numOutputChannels, numSamples);
    auto channelInfo = juce::AudioSourceChannelInfo (&buffer, 0, numSamples);
    audioSource.getNextAudioBlock (channelInfo);
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

    audioSource.prepareToPlay (device->getCurrentBufferSizeSamples(), device->getCurrentSampleRate());
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