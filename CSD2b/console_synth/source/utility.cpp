
// Written by Wouter Ensink

#include <console_synth/utility.h>
#include <juce_events/juce_events.h>

namespace details
{
// uses RAII to start and stop the message thread
ScopedMessageThread::ScopedMessageThread() { juce::initialiseJuce_GUI(); }
ScopedMessageThread::~ScopedMessageThread() { juce::shutdownJuce_GUI(); }

}  // namespace details