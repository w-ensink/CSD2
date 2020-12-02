
#pragma once

#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/audio/audio_callback.h>
#include <console_synth//format.h>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <console_synth/midi/midi_callback.h>


class AudioEngine : public juce::AudioSource
{
public:
    explicit AudioEngine (AudioProcessorBase& rootProcessor);

    ~AudioEngine() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;

private:
    juce::AudioDeviceManager deviceManager {};
    AudioIODeviceCallback audioCallback { *this };

    juce::MidiBuffer midiScratchBuffer;
    AudioProcessorBase& rootProcessor;
    std::unique_ptr<juce::MidiInput> midiInputDevice = nullptr;
    juce::MidiMessageCollector midiMessageCollector;
};
