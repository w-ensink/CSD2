
// Written by Wouter Ensink

#include "format.h"
#include <audio/audio_callback.h>
#include <audio/noise_generator.h>
#include <audio/sine_generator.h>
#include <midi/midi_message_queue.h>
#include <optional>

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
    explicit MidiCallback (juce::MidiMessageCollector& midiMessageCollector) : midiMessageCollector { midiMessageCollector } {}

    void handleIncomingMidiMessage (juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        midiMessageCollector.addMessageToQueue (message);
    }

private:
    juce::MidiMessageCollector& midiMessageCollector;
};


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
        fmt::print("prepare to play, sample rate: {}\n", sampleRate_);
        rootProcessor.prepareToPlay (sampleRate_, samplesPerBlockExpected);
        midiMessageCollector.reset (sampleRate_);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        midiScratchBuffer.clear();
        midiMessageCollector.removeNextBlockOfMessages (midiScratchBuffer, bufferToFill.numSamples);

        auto& buffer = *bufferToFill.buffer;

        rootProcessor.processBlock (buffer, midiScratchBuffer);
    }

    void releaseResources() override
    {
        rootProcessor.releaseResources();
    }


    juce::AudioDeviceManager deviceManager {};
    AudioIODeviceCallback audioCallback { *this };

    juce::MidiBuffer midiScratchBuffer;
    AudioProcessorBase& rootProcessor;
    std::unique_ptr<juce::MidiInput> midiInputDevice = nullptr;
    juce::MidiMessageCollector midiMessageCollector;
    MidiCallback midiCallback { midiMessageCollector };
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
