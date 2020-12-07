

// This test will send a midi message with the midi device juce created
// To make use of this test, you will need some other midi receiving
// program to check if it can find a midi device with the name 'console_synth_midi_output'
// this device should output a midi note with note number 60 on channel 1.

// It has successfully been tested on macOS Big Sur using Max/MSP 8

#include <console_synth/format.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <thread>
#include <console_synth/utility.h>

using namespace std::chrono_literals;

int main()
{
    SCOPE_ENABLE_MESSAGE_THREAD;

    auto name = "console_synth_midi_output";
    // createNewDevice() only works on macOS & Linux (according to juce documentation)
    auto midiOutputDevice = juce::MidiOutput::createNewDevice (name);

    if (! midiOutputDevice)
    {
        fmt::print ("couldn't create midi output device with name {}", name);
        return -1;
    }


    for (;;)
    {
        fmt::print ("enter 'q' to quit, just press enter to send another midi message");

        if (std::cin.get() == 'q')
            break;

        // channels are counted from 1 in juce
        const auto channel = 1;
        const auto noteNumber = 60;

        auto messageNoteOn = juce::MidiMessage::noteOn (channel, noteNumber, 1.0f);
        midiOutputDevice->sendMessageNow (messageNoteOn);

        std::this_thread::sleep_for (100ms);
        auto messageNoteOff = juce::MidiMessage::noteOff (channel, noteNumber);
        midiOutputDevice->sendMessageNow (messageNoteOff);
    }

    return 0;
}