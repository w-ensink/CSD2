
#pragma once

#include <juce_core/juce_core.h>

#define DECLARE_ID(name) \
    static inline const auto name = juce::Identifier { #name }

namespace IDs
{
DECLARE_ID (engine);
DECLARE_ID (sequencer);
DECLARE_ID(tempo);

}  // namespace IDs

#undef DECLARE_ID