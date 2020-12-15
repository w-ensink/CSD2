

#include <console_synth/audio/audio_engine.h>


AudioEngine::AudioEngine (juce::AudioSource& rootAudioSource) : rootAudioSource { rootAudioSource }
{
    auto numInputChannels = 0;
    auto numOutputChannels = 2;
    deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
    deviceManager.addAudioCallback (&audioCallback);
}


AudioEngine::~AudioEngine()
{
    deviceManager.closeAudioDevice();
}


void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    rootAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}


void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    rootAudioSource.getNextAudioBlock (bufferToFill);
}


void AudioEngine::releaseResources()
{
    rootAudioSource.releaseResources();
}