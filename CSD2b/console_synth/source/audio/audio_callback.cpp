
#include <console_synth/audio/audio_callback.h>
#include <console_synth/format.h>
#include <juce_audio_basics/juce_audio_basics.h>


AudioIODeviceCallback::AudioIODeviceCallback (juce::AudioSource& source) : audioSource (source) {};

void AudioIODeviceCallback::audioDeviceIOCallback (const float** inputChannelData,
                                           int numInputChannels,
                                           float** outputChannelData,
                                           int numOutputChannels,
                                           int numSamples)
{
    auto buffer = juce::AudioBuffer<float> (outputChannelData, numOutputChannels, numSamples);
    auto channelInfo = juce::AudioSourceChannelInfo (&buffer, 0, numSamples);
    audioSource.getNextAudioBlock (channelInfo);
}


void AudioIODeviceCallback::audioDeviceAboutToStart (juce::AudioIODevice* device)
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


void AudioIODeviceCallback::audioDeviceStopped()
{
    audioSource.releaseResources();
    fmt::print ("Audio Device Stopped\n");
}


void AudioIODeviceCallback::audioDeviceError (const juce::String& errorMessage)
{
    fmt::print ("Audio Device Error: {}\n", errorMessage);
}