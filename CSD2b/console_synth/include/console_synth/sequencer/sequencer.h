
// main sequencer that is supposed to take an input melody and
// produce events at the right time. Those events are sent to an
// event handler, which can handle it in any desired way.
// This way we can have an event handler that sends midi to the internal
// synth, one that sends it to an external midi device and one that sends
// osc messages or something.

#pragma once

#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/midi/midi_source.h>
#include <console_synth/sequencer/play_head.h>
#include <console_synth/sequencer/track.h>
#include <juce_audio_basics/juce_audio_basics.h>

class Sequencer : public juce::AudioSource
{
public:
    explicit Sequencer (AudioProcessorBase& processor) : processor { processor }
    {
        track.getMelody().notes.push_back (Note {
            .midiNoteNumber = 60,
            .velocity = 100,
            .timeStampTicks = 0,
            .lengthInTicks = 48,
        });

        track.getMelody().notes.push_back (Note {
            .midiNoteNumber = 62,
            .velocity = 100,
            .timeStampTicks = 48,
            .lengthInTicks = 48,
        });

        playHead.setLooping (0, 48 * 2);

        midiSource = std::make_unique<TrackPlayerMidiSource> (track);
    }

    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        setSampleRate (newSampleRate);
    }

    void setSampleRate (double rate)
    {
        sampleRate = rate;
        processor.prepareToPlay (sampleRate, 512);
    }

    void setTempoBpm (double bpm)
    {
        auto ticksPerMinute = bpm * ticksPerQuarterNote;
        auto msPerTick = 60'000 / ticksPerMinute;
        playHead.setTickTimeMs (msPerTick);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        midiBuffer.clear();

        const auto callbackDurationMs = (double) (bufferToFill.numSamples / sampleRate) * 1000;

        if (callbackDurationMs != playHead.getDeviceCallbackDurationMs())
            playHead.setDeviceCallbackDurationMs (callbackDurationMs);

        midiSource->fillNextMidiBuffer (playHead, midiBuffer, bufferToFill.numSamples);

        processor.processBlock (*bufferToFill.buffer, midiBuffer);
        playHead.advanceDeviceBuffer();
    }

    void releaseResources() override {}

private:
    enum struct PlayState
    {
        playing,
        stopped,
        recording
    };

    const uint64_t ticksPerQuarterNote = 48;
    double sampleRate = 0;
    PlayHead playHead;
    PlayState playState = PlayState::stopped;
    Track track;
    std::unique_ptr<MidiSource> midiSource;
    juce::MidiBuffer midiBuffer;
    AudioProcessorBase& processor;
};
