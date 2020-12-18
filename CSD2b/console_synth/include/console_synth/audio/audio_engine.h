
#pragma once

#include <console_synth/audio/audio_callback.h>
#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/identifiers.h>
#include <console_synth/sequencer/sequencer.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

class Engine : private juce::AudioSource
{
public:
    explicit Engine (juce::ValueTree& tree)
    {
        auto numInputChannels = 0;
        auto numOutputChannels = 2;
        deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
        deviceManager.addAudioCallback (&audioCallback);
        tree.addChild (engineState, -1, nullptr);
    }

    ~Engine() override;


    Sequencer& getSequencer() { return sequencer; }

    juce::StringArray getAvailableAudioDevices() const
    {
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        return deviceType->getDeviceNames (false);
    }

    juce::ValueTree getValueTreeState() const
    {
        return engineState;
    }

private:
    juce::ValueTree engineState { IDs::engine };

    // for now the calling of prepare & release resources is in handled by sequencer (should move this around anyway)
    Sequencer sequencer { engineState };
    juce::AudioDeviceManager deviceManager {};
    AudioIODeviceCallback audioCallback { *this };


    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;
};


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