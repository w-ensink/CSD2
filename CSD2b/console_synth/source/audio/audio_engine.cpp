

#include "audio_engine.h"


AudioEngine::AudioEngine (AudioProcessorBase& rootProcessor) : rootProcessor { rootProcessor }
{
    auto numInputChannels = 0;
    auto numOutputChannels = 2;
    deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
    midiScratchBuffer.ensureSize (3000);

    deviceManager.addAudioCallback (&audioCallback);

    auto midiDevices = juce::MidiInput::getAvailableDevices();

    fmt::print ("Opening midi device: {}\n", midiDevices[0].name);
    midiInputDevice = juce::MidiInput::openDevice (midiDevices[0].identifier, &midiCallback);
    midiInputDevice->start();
}


AudioEngine::~AudioEngine()
{
    midiInputDevice->stop();
    deviceManager.closeAudioDevice();
}


void AudioEngine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    rootProcessor.prepareToPlay (sampleRate, samplesPerBlockExpected);
    midiMessageCollector.reset (sampleRate);
}


void AudioEngine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    midiScratchBuffer.clear();
    midiMessageCollector.removeNextBlockOfMessages (midiScratchBuffer, bufferToFill.numSamples);

    auto& buffer = *bufferToFill.buffer;

    rootProcessor.processBlock (buffer, midiScratchBuffer);
}


void AudioEngine::releaseResources()
{
    rootProcessor.releaseResources();
}