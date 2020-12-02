

#include <console_synth/audio/audio_engine.h>


AudioEngine::AudioEngine (AudioProcessorBase& rootProcessor) : rootProcessor { rootProcessor }
{
    auto numInputChannels = 0;
    auto numOutputChannels = 2;
    deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
    midiScratchBuffer.ensureSize (3000);

    deviceManager.addAudioCallback (&audioCallback);

    auto midiDevices = juce::MidiInput::getAvailableDevices();

    fmt::print ("Opening midi device: {}\n", midiDevices[0].name);
    midiInputDevice = juce::MidiInput::openDevice (midiDevices[0].identifier, &midiMessageCollector);
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
    bufferToFill.clearActiveBufferRegion();
    midiScratchBuffer.clear();

    midiMessageCollector.removeNextBlockOfMessages (midiScratchBuffer, bufferToFill.numSamples);
    rootProcessor.processBlock (*bufferToFill.buffer, midiScratchBuffer);
}


void AudioEngine::releaseResources()
{
    rootProcessor.releaseResources();
}