
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/audio_callback.h>
#include <console_synth/utility/format.h>
#include <console_synth/sequencer/sequencer.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

class Engine : private juce::AudioSource
{
public:
    Engine()
    {
        auto numInputChannels = 0;
        auto numOutputChannels = 2;
        deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
        deviceManager.addAudioCallback (&audioCallback);
    }

    ~Engine() override;


    Sequencer& getSequencer() { return sequencer; }

    juce::StringArray getAvailableAudioDevices() const
    {
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        return deviceType->getDeviceNames (false);
    }

    static juce::StringArray getAvailableMidiDevices()
    {
        auto devices = juce::MidiInput::getAvailableDevices();

        auto response = juce::StringArray();

        for (auto& d : devices)
            response.add (fmt::format ("{} ({})", d.name, d.identifier));

        return response;
    }

    juce::ValueTree getValueTreeState() const
    {
        return engineState;
    }

    juce::UndoManager* getUndoManager() { return &undoManager; }

private:

    juce::ValueTree engineState { IDs::engine };
    juce::UndoManager undoManager;
    Sequencer sequencer { engineState };
    juce::AudioDeviceManager deviceManager {};
    AudioIODeviceCallback audioCallback { *this };


    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;
};


