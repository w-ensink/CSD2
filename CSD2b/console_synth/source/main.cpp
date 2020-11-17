
// Written by Wouter Ensink

#include <fmt/format.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>


// partial template specialization to make fmt work with juce::String
template <>
struct fmt::formatter<juce::String>
{
    template <typename ParseContext>
    constexpr auto parse (ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format (const juce::String& toFormat, FormatContext& context)
    {
        return fmt::format_to (context.out(), "{}", toFormat.toRawUTF8());
    }
};


class AudioCallback : public juce::AudioIODeviceCallback
{
public:
    AudioCallback() = default;
    ~AudioCallback() override = default;

    juce::Random random = {};

    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples) override
    {
        for (auto channel = 0; channel < numOutputChannels; ++channel)
        {
            for (auto sample = 0; sample < numSamples; ++sample)
            {
                outputChannelData[channel][sample] = 0.2f * random.nextFloat();
            }
        }
    }

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override
    {
        fmt::print ("{} about to start:\n"
                    "\tsample rate: {}\n"
                    "\tbuffer size: {}\n"
                    "\tbit depth:   {}\n",
                    device->getName(),
                    device->getCurrentSampleRate(),
                    device->getCurrentBufferSizeSamples(),
                    device->getCurrentBitDepth());
    }

    void audioDeviceStopped() override
    {
        fmt::print ("Audio Device Stopped\n");
    }

    void audioDeviceError (const juce::String& errorMessage) override
    {
        fmt::print ("Audio Device Error: {}\n", errorMessage);
    }
};


int main()
{
    auto deviceManager = juce::AudioDeviceManager();
    deviceManager.initialiseWithDefaultDevices (0, 2);
    auto deviceSetup = deviceManager.getAudioDeviceSetup();

    auto callback = AudioCallback();

    deviceManager.addAudioCallback (&callback);

    fmt::print ("Press enter to exit... ");
    std::cin.get();

    deviceManager.closeAudioDevice();
    return 0;
}