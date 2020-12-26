
// Written by Wouter Ensink

#pragma once

#include <console_synth/sequencer/melody.h>
#include <console_synth/sequencer/play_head.h>
#include <juce_audio_basics/juce_audio_basics.h>

class MidiSource
{
public:
    virtual void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& buffer, int numSamples) = 0;
};


class MelodyPlayerMidiSource : public MidiSource
{
public:
    explicit MelodyPlayerMidiSource (Melody* melody) : melody { melody } {}


    void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& buffer, int numSamples) override
    {
        if (melody == nullptr)
            return;

        auto bufferDuration = playHead.getDeviceCallbackDurationMs();

        forEachTick (playHead, [&] (uint64_t tick, double timeStampRelativeToBuffer) {
            melody->forEachEvent ([&] (const auto& event) {
                if (event.timeStampTicks == tick)
                {
                    auto normalizedPosition = timeStampRelativeToBuffer / bufferDuration;
                    auto samplePosition = (int) (normalizedPosition * numSamples);

                    if (event.isNoteOn)
                        buffer.addEvent (juce::MidiMessage::noteOn (1, event.midiNote, (uint8_t) event.velocity), samplePosition);
                    else
                        buffer.addEvent (juce::MidiMessage::noteOff (1, event.midiNote, (uint8_t) event.velocity), samplePosition);
                }
            });
        });
    }

private:
    Melody* melody;
};