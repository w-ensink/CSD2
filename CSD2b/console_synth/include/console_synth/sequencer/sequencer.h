
// main sequencer that is supposed to take an input melody and
// produce events at the right time. Those events are sent to an
// event handler, which can handle it in any desired way.
// This way we can have an event handler that sends midi to the internal
// synth, one that sends it to an external midi device and one that sends
// osc messages or something.

#pragma once

#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/midi/midi_source.h>
#include <console_synth/property.h>
#include <console_synth/sequencer/play_head.h>
#include <console_synth/sequencer/time_signature.h>
#include <console_synth/sequencer/track.h>
#include <juce_audio_basics/juce_audio_basics.h>


enum struct PlayState
{
    playing,
    stopped,
    recording
};


struct RenderContext final
{
public:
    RenderContext (juce::AudioBuffer<float>& audioBuffer,
                   const juce::MidiBuffer& externalMidi,
                   const PlayHead& playHead,
                   PlayState playState,
                   double sampleRate,
                   juce::Range<double> deviceStreamTimeSpanMs,
                   const TimeSignature& timeSignature)
        : destinationAudioBuffer { audioBuffer },
          externalMidi { externalMidi },
          playHead { playHead },
          playState { playState },
          sampleRate { sampleRate },
          deviceStreamTimeSpan { deviceStreamTimeSpanMs },
          timeSignature { timeSignature }
    {
    }

    [[nodiscard]] const PlayHead& getPlayHead() const noexcept { return playHead; }

    void addToAudioBuffer (int channel, int sample, float value)
    {
        destinationAudioBuffer.addSample (channel, sample, value);
    }

    [[nodiscard]] bool isRecording() const noexcept { return playState == PlayState::recording; }
    [[nodiscard]] bool isPlaying() const noexcept { return playState == PlayState::playing; }
    [[nodiscard]] bool isStopped() const noexcept { return playState == PlayState::stopped; }

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate; }

    [[nodiscard]] const juce::MidiBuffer& getExternalMidi() const noexcept { return externalMidi; }

    juce::AudioBuffer<float>& getAudioBuffer() noexcept { return destinationAudioBuffer; }

    [[nodiscard]] const TimeSignature& getTimeSignature() const noexcept { return timeSignature; }

private:
    juce::AudioBuffer<float>& destinationAudioBuffer;
    const juce::MidiBuffer& externalMidi;
    const PlayHead& playHead;
    PlayState playState;
    double sampleRate;
    juce::Range<double> deviceStreamTimeSpan;
    const TimeSignature& timeSignature;
};


namespace dev
{
struct Track
{
    void renderNextBlock (RenderContext& renderContext) {}

    // has melody
    // has processor chain
    // has bool record enabled
    // has a way to record external midi messages from the render context into the melody
    // has a name
};

}  // namespace dev


class Sequencer : public juce::AudioSource
{
public:
    explicit Sequencer (AudioProcessorBase& processor, juce::ValueTree& parent) : processor { processor }
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
        setTempoBpm (tempoBpm.getValue());

        parent.addChild (sequencerState, -1, nullptr);

        tempoBpm.onChange = [this] (auto newTempo) {
            setTempoBpm (newTempo);
        };
    }

    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        setSampleRate (newSampleRate);
    }


    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        // ensure the play head has the right callback duration set
        const auto callbackDurationMs = (double) (bufferToFill.numSamples / sampleRate) * 1000;

        if (callbackDurationMs != playHead.getDeviceCallbackDurationMs())
            playHead.setDeviceCallbackDurationMs (callbackDurationMs);

        // clear the destination buffer for the external midi messages
        midiBuffer.clear();

        // fetch the incoming midi messages from external midi

        // prepare the render context for the current render pass
        auto renderContext = RenderContext {
            *bufferToFill.buffer,
            midiBuffer,
            playHead,
            playState,
            sampleRate,
            { 0, callbackDurationMs },
            timeSignature
        };

        // render over all tracks...
        midiSource->fillNextMidiBuffer (playHead, midiBuffer, bufferToFill.numSamples);
        processor.processBlock (*bufferToFill.buffer, midiBuffer);

        // move play head one block ahead to prepare for the next callback
        playHead.advanceDeviceBuffer();
    }

    void releaseResources() override {}

private:
    juce::ValueTree sequencerState { IDs::sequencer };
    Property<double> tempoBpm { sequencerState, IDs::tempo, nullptr, 100 };
    TimeSignature timeSignature { 4, 4, 48 };
    double sampleRate = 0;
    PlayHead playHead;
    PlayState playState = PlayState::stopped;
    Track track;
    std::unique_ptr<MidiSource> midiSource;
    juce::MidiBuffer midiBuffer;
    AudioProcessorBase& processor;


    void setTempoBpm (double bpm)
    {
        auto ticksPerMinute = bpm * timeSignature.getTicksPerQuarterNote();
        auto msPerTick = 60'000 / ticksPerMinute;
        playHead.setTickTimeMs (msPerTick);
    }


    void setSampleRate (double rate)
    {
        sampleRate = rate;
        processor.prepareToPlay (sampleRate, 512);
    }
};
