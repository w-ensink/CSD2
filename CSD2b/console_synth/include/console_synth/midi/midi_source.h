
#pragma once


#include <console_synth/sequencer/play_head.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <console_synth/sequencer/event.h>

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
        auto&& events = melody->getSequencerEvents();

        forEachTick (playHead, [&] (uint64_t tick, double timeStampRelativeToBuffer) {
            for (auto&& e : events)
            {
                if (e.timeStampTicks == tick)
                {
                    auto normalizedPosition = timeStampRelativeToBuffer / bufferDuration;
                    auto samplePosition = (int) (normalizedPosition * numSamples);

                    if (e.isNoteOn)
                        buffer.addEvent (juce::MidiMessage::noteOn (1, e.midiNote, e.velocity), samplePosition);
                    else
                        buffer.addEvent (juce::MidiMessage::noteOff (1, e.midiNote, e.velocity), samplePosition);
                }
            }
        });
    }

private:
    Melody* melody;
};