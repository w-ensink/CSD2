
// Written by Wouter Ensink

#include "format.h"
#include <audio/audio_callback.h>
#include <audio/noise_generator.h>
#include <audio/sine_generator.h>
#include <midi/midi_message_queue.h>

auto startAudioPlayback()
{
    auto deviceManager = juce::AudioDeviceManager();
    deviceManager.initialiseWithDefaultDevices (0, 2);
    auto deviceSetup = deviceManager.getAudioDeviceSetup();

    // auto noiseGenerator = NoiseGenerator();
    auto sineGenerator = SineGenerator();
    //  auto callback = AudioCallback (sineGenerator);

    //  deviceManager.addAudioCallback (&callback);

    fmt::print ("Press enter to exit... ");
    std::cin.get();

    deviceManager.closeAudioDevice();
}

// uses RAII to start and stop the message thread
struct ScopedMessageThread
{
    ScopedMessageThread() { juce::initialiseJuce_GUI(); }
    ~ScopedMessageThread() { juce::shutdownJuce_GUI(); }
};

#define SCOPE_ENABLE_MESSAGE_THREAD \
    ScopedMessageThread smt

// ==============================================================

class MidiCallback : public juce::MidiInputCallback
{
public:
    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        if (message.isNoteOff())
            return;

        const auto& m = message;

        fmt::print ("Message from: {}\n"
                    "\tnote number: {}\n"
                    "\tchannel:     {}\n"
                    "\ttime stamp:  {}\n"
                    "\tsize in bytes {}\n",
                    source->getName(),
                    m.getNoteNumber(),
                    m.getChannel(),
                    m.getTimeStamp(),
                    m.getRawDataSize());
    }
};


// =====================================================

struct AudioStreamInfo
{
    int numSamplesPerBlock;
    int streamTimeSamples;
    int bufferRelativeTimeSamples;
};

// when midi messages are added to this class, it converts the time
// when it was added to streaming time.
// it then puts it in the MidiBuffer given by the audio engine

auto getCurrentTimePoint() noexcept
{
    return std::chrono::high_resolution_clock::now();
}


class MidiScheduler
{
public:
    void addMessage (const juce::MidiMessage& message)
    {
        auto now = getCurrentTimePoint();
        auto timeSinceLastBuffer = std::chrono::duration_cast<std::chrono::milliseconds> (now - lastBufferStartTime).count();
    }

    void fillMidiBuffer (juce::MidiBuffer& bufferToFill, const AudioStreamInfo& streamInfo)
    {
        lastBufferStartTime = getCurrentTimePoint();
        bufferToFill.clear();
    }

    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
    TimePoint lastBufferStartTime;
    double sampleRate;
};


// =====================================================

struct AudioEngine : juce::AudioSource
{
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate_) override
    {
        audioStreamInfo.streamTimeSamples = 0;
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        // increase the current time stamp for the next callback
        audioStreamInfo.streamTimeSamples += bufferToFill.numSamples;
    }


    AudioStreamInfo audioStreamInfo;
};


int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;

    auto devices = juce::MidiInput::getAvailableDevices();

    for (auto& d : devices)
    {
        fmt::print ("name: {}, identifier: {}\n", d.name, d.identifier);
    }

    auto midiCallback = MidiCallback();
    fmt::print ("opening {}\n", devices[0].identifier);
    auto dev = juce::MidiInput::openDevice (devices[0].identifier, &midiCallback);
    dev->start();

    fmt::print ("press enter to exit... \n");
    std::cin.get();
    dev->stop();

    return 0;
}