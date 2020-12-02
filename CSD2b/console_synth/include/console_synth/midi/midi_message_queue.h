
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

// lock free queue for midi messages
// in the bigger picture, messages will be added from the message thread
// and will then be handled from the audio thread
class MidiMessageQueue
{
public:
    explicit MidiMessageQueue (std::size_t size) : fifo (size), buffer (size) {}

    void addMessage (const juce::MidiMessage& message)
    {
        fifo.write (1).forEach ([this, &message] (auto index) {
            buffer[index] = message;
        });
    }

    template <typename MessageHandler>
    void handleAllPendingMessages (MessageHandler&& handler)
    {
        fifo.read (fifo.getNumReady()).forEach ([this, &handler] (auto index) {
            auto message = buffer[index];
            handler (message);
        });
    }

private:
    juce::AbstractFifo fifo;
    std::vector<juce::MidiMessage> buffer;
};