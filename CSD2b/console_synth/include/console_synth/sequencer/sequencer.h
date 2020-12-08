
// main sequencer that is supposed to take an input melody and
// produce events at the right time. Those events are sent to an
// event handler, which can handle it in any desired way.
// This way we can have an event handler that sends midi to the internal
// synth, one that sends it to an external midi device and one that sends
// osc messages or something.

#pragma once

#include <console_synth/sequencer/clock.h>
#include <console_synth/sequencer/playhead.h>


class Sequencer : public juce::Thread
{
public:
    Sequencer() : juce::Thread { "Sequencer Thread" } {}


    void startPlayback()
    {
        playState = PlayState::playing;
        clock->start();
    }

    void stopPlayback()
    {
        playState = PlayState::stopped;
    }


    // the reason for this function, is to inject a different clock
    // during testing you don't want the tests to take forever for example
    void setCustomClock (std::unique_ptr<SequencerClockBase> customClock)
    {
        clock = std::move (customClock);
    }

    void run() override
    {
    }


private:
    enum struct PlayState
    {
        playing,
        stopped
    };

    std::unique_ptr<SequencerClockBase> clock = std::make_unique<Clock> (0ms);
    PlayHead playHead;
    PlayState playState = PlayState::stopped;
};
