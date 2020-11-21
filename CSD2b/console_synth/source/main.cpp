
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
    explicit MidiCallback (MidiMessageQueue& messageDestinationQueue) : messageDestinationQueue { messageDestinationQueue } {}

    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        messageDestinationQueue.addMessage (message);
    }

private:
    MidiMessageQueue& messageDestinationQueue;
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


// =====================================================

struct AudioEngine : juce::AudioSource
{
    explicit AudioEngine (AudioProcessorBase& rootProcessor) : rootProcessor { rootProcessor }
    {
        auto numInputChannels = 0;
        auto numOutputChannels = 2;
        deviceManager.initialiseWithDefaultDevices (numInputChannels, numOutputChannels);
        midiScratchBuffer.ensureSize (3000);

        deviceManager.addAudioCallback (&audioCallback);

        auto midiDevices = juce::MidiInput::getAvailableDevices();

        fmt::print ("Opening midi device: {}\n", midiDevices[0].name);
        midiInputDevice = juce::MidiInput::openDevice (midiDevices[0].identifier, &midiCallback);
        midiInputDevice->start();
    }

    ~AudioEngine() override
    {
        midiInputDevice->stop();
        deviceManager.closeAudioDevice();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate_) override
    {
        rootProcessor.prepareToPlay (sampleRate_, samplesPerBlockExpected);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        midiScratchBuffer.clear();

        midiMessageQueue.handleAllPendingMessages ([this] (auto message) {
            midiScratchBuffer.addEvent (message, 0);
        });

        auto& buffer = *bufferToFill.buffer;

        rootProcessor.processBlock (buffer, midiScratchBuffer);
    }

    void releaseResources() override
    {
        rootProcessor.releaseResources();
    }


    juce::AudioDeviceManager deviceManager {};
    MidiMessageQueue midiMessageQueue { 1000 };
    AudioIODeviceCallback audioCallback { *this };
    MidiCallback midiCallback { midiMessageQueue };
    juce::MidiBuffer midiScratchBuffer;
    AudioProcessorBase& rootProcessor;
    std::unique_ptr<juce::MidiInput> midiInputDevice = nullptr;
};


int main()
{
    // keep the juce message thread alive during the whole main function
    // to enable midi and osc
    SCOPE_ENABLE_MESSAGE_THREAD;

    auto rootProcessor = SineGenerator();
    auto engine = AudioEngine (rootProcessor);

    fmt::print ("press enter to exit... \n");
    std::cin.get();

    return 0;
}
