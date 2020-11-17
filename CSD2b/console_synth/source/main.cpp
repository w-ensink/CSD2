
// Written by Wouter Ensink

#include "format.h"
#include <audio_callback.h>
#include <noise_generator.h>

int main()
{
    auto deviceManager = juce::AudioDeviceManager();
    deviceManager.initialiseWithDefaultDevices (0, 2);
    auto deviceSetup = deviceManager.getAudioDeviceSetup();

    auto noiseGenerator = NoiseGenerator();
    auto callback = AudioCallback (noiseGenerator);

    deviceManager.addAudioCallback (&callback);

    fmt::print ("Press enter to exit... ");
    std::cin.get();

    deviceManager.closeAudioDevice();
    return 0;
}