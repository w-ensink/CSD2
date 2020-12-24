
#pragma once

#include <juce_core/juce_core.h>

#define DECLARE_ID(name) \
    static inline const auto name = juce::Identifier { #name }

namespace IDs
{
DECLARE_ID (engine);
DECLARE_ID (sequencer);
DECLARE_ID (tempo);
DECLARE_ID (note);
DECLARE_ID (midiNoteNumber);
DECLARE_ID (velocity);
DECLARE_ID (startTimeTicks);
DECLARE_ID (lengthTicks);
DECLARE_ID (melody);
DECLARE_ID (track);

}  // namespace IDs

#undef DECLARE_ID