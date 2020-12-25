
// Written by Wouter Ensink

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <fmt/format.h>


class MidiCallback : public juce::MidiInputCallback
{
public:
    explicit MidiCallback (juce::MidiMessageCollector& midiMessageCollector) : midiMessageCollector { midiMessageCollector } {}

    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        midiMessageCollector.addMessageToQueue (message);
    }

private:
    juce::MidiMessageCollector& midiMessageCollector;
};
