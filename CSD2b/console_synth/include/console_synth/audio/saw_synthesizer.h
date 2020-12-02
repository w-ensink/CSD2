

#pragma once

#include "audio_processor_base.h"
#include <juce_audio_processors/juce_audio_processors.h>

class SawSynthesizerSound : public juce::SynthesiserSound
{
public:
    bool appliesToChannel (int midiChannel) override { return true; }
    bool appliesToNote (int midiNoteNumber) override { return true; }
};


class SawSynthesizerVoice : public juce::SynthesiserVoice
{
public:
    SawSynthesizerVoice()
    {
        auto envParams = juce::ADSR::Parameters {
            .attack = 0.01,
            .decay = 0.5,
            .sustain = 0.6,
            .release = 0.2
        };

        envelope.setSampleRate (getSampleRate());
        envelope.setParameters (envParams);
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        for (auto sample = startSample; sample < numSamples + startSample; ++sample)
        {
            for (auto channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.addSample (channel, sample, getNextSample());
            }
        }
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        envelope.noteOff();
        clearCurrentNote();
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
        envelope.reset();
        envelope.noteOn();
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SawSynthesizerSound*> (sound) != nullptr;
    }

    void controllerMoved (int controllerNumber, int newControllerValue) override {}
    void pitchWheelMoved (int newPitchWheelValue) override {}

private:
    juce::ADSR envelope;

    double phase = 0.0;
    double angleDelta = 0.0;
    double frequency = 0.0;
    double amplitude = 0.2;

    double getNextSample()
    {
        auto value = (phase * 2) - 1;
        phase += angleDelta;
        phase = fmod (phase, 1.0);
        return value * amplitude * envelope.getNextSample();
    }

    void setFrequency (double freq)
    {
        frequency = freq;
        auto cyclesPerSample = frequency / getSampleRate();
        angleDelta = cyclesPerSample;
    }
};


// ==========================================================================================================


class SawSynthesizer : public AudioProcessorBase
{
public:
    explicit SawSynthesizer (int numVoices)
    {
        for (auto i = 0; i < numVoices; ++i)
            synthEngine.addVoice (new SawSynthesizerVoice());

        synthEngine.addSound (new SawSynthesizerSound());
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
