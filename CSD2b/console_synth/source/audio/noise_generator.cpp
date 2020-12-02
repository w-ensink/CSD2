
#include <console_synth/audio/noise_generator.h>


void NoiseGenerator::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* writePointer = buffer.getWritePointer (channel);

        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            writePointer[sample] = 0.1f * random.nextFloat();
        }
    }
}