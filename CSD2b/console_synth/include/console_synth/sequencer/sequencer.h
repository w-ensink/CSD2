
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/identifiers.h>
#include <console_synth/sequencer/play_head.h>
#include <console_synth/sequencer/time_signature.h>
#include <console_synth/sequencer/track.h>
#include <console_synth/utility/property.h>
#include <juce_audio_devices/juce_audio_devices.h>


class Sequencer : public juce::AudioSource
{
public:
    explicit Sequencer (juce::ValueTree& parent);
    ~Sequencer() override = default;

    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override;


    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;


    void releaseResources() override;


    bool openMidiInputDevice (const juce::String& name);

    void stopPlayback();

    void startPlayback();

    [[nodiscard]] bool isPlaying() const noexcept;

    const TimeSignature& getTimeSignature() const noexcept;


private:
    juce::ValueTree sequencerState { IDs::sequencer };
    Property<double> tempoBpm { sequencerState, IDs::tempo, 100 };
    TimeSignature timeSignature { 4, 4, 48 };
    double sampleRate = 0;
    PlayHead playHead;
    PlayState playState = PlayState::stopped;
    Track track { sequencerState };
    juce::MidiMessageCollector midiMessageCollector;
    std::unique_ptr<juce::MidiInput> currentMidiInput = nullptr;
    juce::MidiBuffer midiBuffer;


    void setTempoBpm (double bpm);
    void setSampleRate (double rate);
};
