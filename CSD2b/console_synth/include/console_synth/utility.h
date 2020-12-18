
#pragma once

#include <juce_events/juce_events.h>

namespace details
{
// uses RAII to start and stop the message thread
struct ScopedMessageThread
{
    ScopedMessageThread() { juce::initialiseJuce_GUI(); }
    ~ScopedMessageThread() { juce::shutdownJuce_GUI(); }
};

#define SCOPE_ENABLE_MESSAGE_THREAD \
    auto smt = details::ScopedMessageThread {}

}  // namespace details


