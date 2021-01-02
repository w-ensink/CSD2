
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/synth_types.h>
#include <console_synth/audio/synthesizer.h>
#include <console_synth/midi/midi_source.h>
#include <console_synth/sequencer/melody.h>
#include <console_synth/sequencer/render_context.h>

struct Track
{
    explicit Track (juce::ValueTree parent);
    ~Track() = default;

    void prepareToPlay (double newSampleRate, int numSamplesPerBlockExpected);

    void renderNextBlock (RenderContext& renderContext);

    void releaseResources();

private:
    juce::ValueTree trackState { IDs::track };
    std::mutex synthMutex;
    Property<SynthType> synthType { trackState, IDs::synthType, SynthType::fm };
    std::unique_ptr<SynthesizerBase> synth = std::make_unique<FmSynthesizer> (trackState);
    std::bitset<128> activeMidiNotes { 0 };
    Melody melody { trackState };
    MelodyPlayerMidiSource melodyPlayerMidiSource { &melody };
    bool isRecordEnabled = true;
    juce::MidiBuffer midiScratchBuffer;
    PlayState previousPlayState = PlayState::stopped;
    double sampleRate = 44100.0;


    template <typename SynthType>
    void switchSynth()
    {
        auto synthLock = std::scoped_lock { synthMutex };
        auto synthNode = trackState.getChildWithName (IDs::synth);
        trackState.removeChild (synthNode, nullptr);
        auto newSynth = std::make_unique<SynthType> (trackState);
        newSynth->prepareToPlay (sampleRate, 512);
        synth = std::move (newSynth);
    }

    void addInternalMidiToScratchBuffer (const RenderContext& renderContext);

    void addExternalMidiToScratchBuffer (const RenderContext& renderContext);

    void updateActiveMidiNotes();

    void addMidiOffMessagesForActiveMidiNotes (int sample);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Track);
};
