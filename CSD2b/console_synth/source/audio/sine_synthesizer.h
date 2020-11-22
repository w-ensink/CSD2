
#pragma once

#include "audio_processor_base.h"
#include <juce_audio_processors/juce_audio_processors.h>


class SineSynthesizerSound : public juce::SynthesiserSound
{
public:
    bool appliesToChannel (int midiChannel) override { return true; }
    bool appliesToNote (int midiNoteNumber) override { return true; }
};

// ==========================================================================================================

class SineSynthesizerVoice : public juce::SynthesiserVoice
{
public:
    SineSynthesizerVoice()
    {
        auto envParams = juce::ADSR::Parameters {
            .attack = 0.001,
            .decay = 0.2,
            .sustain = 0.1,
            .release = 0.1
        };

        envelope.setSampleRate (getSampleRate());
        envelope.setParameters (envParams);
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        for (auto sample = startSample; sample < numSamples + startSample; ++sample)
        {
            const auto value = sin (phase) * envelope.getNextSample() * amplitude;
            phase += angleDelta;

            for (auto channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                // addSample mixes the existing sample with the new one, so no override
                outputBuffer.addSample (channel, sample, value);
            }
        }
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
        envelope.reset();
        envelope.noteOn();
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        envelope.noteOff();
        clearCurrentNote();
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineSynthesizerSound*> (sound) != nullptr;
    }

    void pitchWheelMoved (int newPitchWheelValue) override {}
    void controllerMoved (int controllerNumber, int newControllerValue) override {}

private:
    juce::ADSR envelope;
    double phase = 0.0;
    double angleDelta = 0.0;
    double amplitude = 0.2;
    double frequency = 0.0;

    void setFrequency (double freq)
    {
        frequency = freq;
        auto cyclesPerSample = frequency / getSampleRate();
        angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    }
};


// ==========================================================================================================


class SineSynthesizer : public AudioProcessorBase
{
public:
    explicit SineSynthesizer (int numVoices)
    {
        for (auto i = 0; i < numVoices; ++i)
            synthEngine.addVoice (new SineSynthesizerVoice());

        synthEngine.addSound (new SineSynthesizerSound());
    }

    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
        synthEngine.setCurrentPlaybackSampleRate (sampleRate);
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        synthEngine.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
    }

    void releaseResources() override
    {
        synthEngine.clearSounds();
        synthEngine.clearVoices();
    }


private:
    juce::Synthesiser synthEngine;
};
