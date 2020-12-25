
// Written by Wouter Ensink

#include <console_synth/engine.h>

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
