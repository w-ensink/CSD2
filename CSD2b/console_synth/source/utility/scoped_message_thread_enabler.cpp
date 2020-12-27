
// Written by Wouter Ensink

#include <console_synth/utility/scoped_message_thread_enabler.h>
#include <juce_events/juce_events.h>

namespace details
{
// uses RAII to start and stop the message thread
ScopedMessageThreadEnabler::ScopedMessageThreadEnabler() { juce::initialiseJuce_GUI(); }
ScopedMessageThreadEnabler::~ScopedMessageThreadEnabler() { juce::shutdownJuce_GUI(); }

}  // namespace details