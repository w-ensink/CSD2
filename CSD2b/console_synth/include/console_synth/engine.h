
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/audio_callback.h>
#include <console_synth/sequencer/sequencer.h>
#include <console_synth/utility/format.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

class Engine : private juce::AudioSource
{
public:
    Engine();
    ~Engine() override;

    Sequencer& getSequencer();

    juce::StringArray getAvailableAudioDevices() const;

    static juce::StringArray getAvailableMidiDevices();

    juce::ValueTree getValueTreeState() const;

    juce::UndoManager* getUndoManager();

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
