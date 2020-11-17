
#include "noise_generator.h"


void NoiseGenerator::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    const auto [buffer, startSample, numSamples] = bufferToFill;

    for (auto channel = 0; channel < buffer->getNumChannels(); ++channel)
    {
        auto* writePointer = buffer->getWritePointer (channel);

        for (auto sample = startSample; sample < numSamples + startSample; ++sample)
        {
            writePointer[sample] = 0.1f * random.nextFloat();
        }
    }
}