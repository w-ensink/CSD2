
// main sequencer that is supposed to take an input melody and
// produce events at the right time. Those events are sent to an
// event handler, which can handle it in any desired way.
// This way we can have an event handler that sends midi to the internal
// synth, one that sends it to an external midi device and one that sends
// osc messages or something.

#pragma once

#include <console_synth/sequencer/clock.h>
#include <console_synth/sequencer/playhead.h>


class Sequencer
{
public:

    void setSampleRate(double sampleRate)
    {

    }

    void setTempoBpm(double bpm)
    {

    }

private:
    enum struct PlayState
    {
        playing,
        stopped,
        recording
    };

    PlayHead playHead;
    PlayState playState = PlayState::stopped;
};
