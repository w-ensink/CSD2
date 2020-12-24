

// structure that represents an event that can be scheduled by the sequencer
//

#pragma once

#include <console_synth/sequencer/time_signature.h>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

struct Event
{
    int midiNote;
    int velocity;
    int timeStampTicks;
    bool isNoteOn;
};


