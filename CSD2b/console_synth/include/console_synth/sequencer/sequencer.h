
// Written by Wouter Ensink

#pragma once

#include <console_synth/audio/audio_processor_base.h>
#include <console_synth/identifiers.h>
#include <console_synth/utility/property.h>
#include <console_synth/sequencer/play_head.h>
#include <console_synth/sequencer/time_signature.h>
#include <console_synth/sequencer/track.h>
#include <juce_audio_devices/juce_audio_devices.h>


class Sequencer : public juce::AudioSource
{
public:
    explicit Sequencer (juce::ValueTree& parent)
    {
        playHead.setLooping (0, 48 * 4 + 1);

        setTempoBpm (tempoBpm.getValue());

        parent.addChild (sequencerState, -1, nullptr);

        tempoBpm.onChange = [this] (auto newTempo) { setTempoBpm (newTempo); };

        midiMessageCollector.ensureStorageAllocated (256);
    }

    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        setSampleRate (newSampleRate);
        track.prepareToPlay (sampleRate, samplesPerBlockExpected);
        midiMessageCollector.reset (sampleRate);
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
        midiMessageCollector.removeNextBlockOfMessages (midiBuffer, bufferToFill.numSamples);

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
        track.renderNextBlock (renderContext);

        // move play head one block ahead to prepare for the next callback
        if (playState != PlayState::stopped)
            playHead.advanceDeviceBuffer();
    }

    void releaseResources() override
    {
        track.releaseResources();
    }

    bool openMidiInputDevice (const juce::String& name)
    {
        for (auto& d : juce::MidiInput::getAvailableDevices())
        {
            if (d.name.equalsIgnoreCase (name))
            {
                if (auto newInput = juce::MidiInput::openDevice (d.identifier, &midiMessageCollector))
                {
                    if (currentMidiInput != nullptr)
                        currentMidiInput->stop();

                    currentMidiInput = std::move (newInput);
                    currentMidiInput->start();
                    return true;
                }
            }
        }

        return false;
    }

    void stopPlayback()
    {
        playState = PlayState::stopped;
    }

    void startPlayback()
    {
        playState = PlayState::playing;
    }

    const TimeSignature& getTimeSignature() const noexcept { return timeSignature; }

private:
    juce::ValueTree sequencerState { IDs::sequencer };
    Property<double> tempoBpm { sequencerState, IDs::tempo, nullptr, 100 };
    TimeSignature timeSignature { 4, 4, 48 };
    double sampleRate = 0;
    PlayHead playHead;
    PlayState playState = PlayState::stopped;
    Track track { sequencerState };
    juce::MidiMessageCollector midiMessageCollector;
    std::unique_ptr<juce::MidiInput> currentMidiInput = nullptr;
    juce::MidiBuffer midiBuffer;


    void setTempoBpm (double bpm)
    {
        auto ticksPerMinute = bpm * timeSignature.getTicksPerQuarterNote();
        auto msPerTick = 60'000 / ticksPerMinute;
        playHead.setTickTimeMs (msPerTick);
    }


    void setSampleRate (double rate)
    {
        sampleRate = rate;
    }
};
