
#pragma once

#include "audio_processor_base.h"
#include <juce_audio_processors/juce_audio_processors.h>


class SineGenerator : public AudioProcessorBase
{
public:
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        for (auto m : midiMessages)
        {
            auto message = m.getMessage();
            if (message.isNoteOn())
            {
                setFrequency (juce::MidiMessage::getMidiNoteInHertz (message.getNoteNumber()));
                amplitude = 0.1;
            }
            else
                amplitude = 0.0;
        }

        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            const auto value = sin (phase);
            phase += angleDelta;

            for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                auto* writePointer = buffer.getWritePointer (channel);
                writePointer[sample] = value * amplitude;
            }
        }
    }

    void prepareToPlay (double sampleRate_, int maximumExpectedSamplesPerBlock) override
    {
        sampleRate = sampleRate_;
        updateAngleDelta();
    }

    void setFrequency (double newFrequency)
    {
        frequency = newFrequency;
        updateAngleDelta();
    }

private:
    double phase = 0.0;
    double frequency = 440.0;
    double angleDelta = 0.0;
    double sampleRate = 44100.0;
    double amplitude = 0.1;

    void updateAngleDelta()
    {
        auto cyclesPerSample = frequency / sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    }
};