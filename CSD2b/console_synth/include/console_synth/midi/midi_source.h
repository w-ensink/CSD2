
#pragma once


#include <console_synth/sequencer/play_head.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <console_synth/sequencer/track.h>

class MidiSource
{
public:
    virtual void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& buffer, int numSamples) = 0;
};



class TrackPlayerMidiSource : public MidiSource
{
public:
    TrackPlayerMidiSource (Track& track) : track { track } {}

    Track& getTrack() { return track; }
    void setTrack (Track& track_) { track = track_; }


    void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& buffer, int numSamples) override
    {
        auto bufferDuration = playHead.getDeviceCallbackDurationMs();
        auto&& events = track.getMelody().getSequencerEvents();

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
    Track& track;
};