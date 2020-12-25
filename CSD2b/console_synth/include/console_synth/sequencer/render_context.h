
// Written by Wouter Ensink

#pragma once

#include <console_synth/sequencer/play_head.h>
#include <console_synth/sequencer/time_signature.h>
#include <juce_audio_basics/juce_audio_basics.h>

enum struct PlayState
{
    playing,
    stopped,
    recording,
    exporting
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
    [[nodiscard]] bool isExporting() const noexcept { return playState == PlayState::exporting; }

    [[nodiscard]] int getNumSamples() const noexcept { return destinationAudioBuffer.getNumSamples(); }

    [[nodiscard]] double getSampleRate() const noexcept { return sampleRate; }

    [[nodiscard]] const juce::MidiBuffer& getExternalMidi() const noexcept { return externalMidi; }

    juce::AudioBuffer<float>& getAudioBuffer() noexcept { return destinationAudioBuffer; }

    [[nodiscard]] const TimeSignature& getTimeSignature() const noexcept { return timeSignature; }

    [[nodiscard]] PlayState getPlayState() const noexcept { return playState; }

private:
    juce::AudioBuffer<float>& destinationAudioBuffer;
    const juce::MidiBuffer& externalMidi;
    const PlayHead& playHead;
    PlayState playState;
    double sampleRate;
    juce::Range<double> deviceStreamTimeSpan;
    const TimeSignature& timeSignature;
};
