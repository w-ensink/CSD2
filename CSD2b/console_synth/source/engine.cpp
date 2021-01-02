
// Written by Wouter Ensink

#include <console_synth/engine.h>


Engine::Engine()
{
    auto numInputChannels = 0;
    auto numOutputChannels = 2;
    deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
    deviceManager.addAudioCallback (&audioCallback);
}


Engine::~Engine()
{
    deviceManager.closeAudioDevice();
}


void Engine::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    sequencer.prepareToPlay (samplesPerBlockExpected, sampleRate);
}


void Engine::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    sequencer.getNextAudioBlock (bufferToFill);
}


void Engine::releaseResources()
{
    sequencer.releaseResources();
}

Sequencer& Engine::getSequencer() { return sequencer; }

juce::StringArray Engine::getAvailableAudioDevices() const
{
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    return deviceType->getDeviceNames (false);
}

juce::StringArray Engine::getAvailableMidiDevices()
{
    auto devices = juce::MidiInput::getAvailableDevices();

    auto response = juce::StringArray();

    for (auto& d : devices)
        response.add (fmt::format ("{} ({})", d.name, d.identifier));

    return response;
}

juce::ValueTree Engine::getValueTreeState() const
{
    return engineState;
}

juce::UndoManager* Engine::getUndoManager() { return &undoManager; }