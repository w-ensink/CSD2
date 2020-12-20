
#pragma once

#include "audio_processor_base.h"
#include <console_synth/audio/oscillator.h>


class GeneralSynthesizerVoice : public juce::SynthesiserSound
{
public:
    bool appliesToChannel (int midiChannel) override { return true; }
    bool appliesToNote (int midiNoteNumber) override { return true; }
};


template <typename OscillatorType>
class OscillatorSynthesizerVoice : public juce::SynthesiserVoice
{
public:
    OscillatorSynthesizerVoice()
    {
        auto envParams = juce::ADSR::Parameters {
            .attack = 0.001,
            .decay = 0.5,
            .sustain = 0.4,
            .release = 0.2
        };

        envelope.setSampleRate (getSampleRate());
        envelope.setParameters (envParams);

        oscillator.setSampleRate (getSampleRate());
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        for (auto sample = startSample; sample < numSamples + startSample; ++sample)
        {
            auto value = getNextSample();

            for (auto channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.addSample (channel, sample, value);
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
        oscillator.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
        envelope.reset();
        envelope.noteOn();
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<GeneralSynthesizerVoice*> (sound) != nullptr;
    }

    void controllerMoved (int controllerNumber, int newControllerValue) override {}
    void pitchWheelMoved (int newPitchWheelValue) override {}

    auto& getOscillator()
    {
        return oscillator;
    }

    // oscillator controller will be a templated class,
    // with partial specialization for each different type of oscillator
    auto getOscillatorController()
    {
    }

private:
    OscillatorType oscillator;
    juce::ADSR envelope;


    double getNextSample()
    {
        oscillator.advance();
        return oscillator.getSample() * envelope.getNextSample();
    }
};


class SynthesizerBase : public AudioProcessorBase
{
public:
    SynthesizerBase()
    {
        synthEngine.addSound (new GeneralSynthesizerVoice());
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

protected:
    juce::Synthesiser synthEngine;
};


// fm synth with anti aliased (8x oversampled) voices with 1 sine carrier and 3 sine modulators
class FmSynthesizer : public SynthesizerBase
{
public:
    using OscType = AntiAliased<FmOsc<SineOsc<float>, SineOsc<float>, SineOsc<float>, SineOsc<float>>>;
    using VoiceType = OscillatorSynthesizerVoice<OscType>;

    explicit FmSynthesizer (int numVoices)
    {
        for (auto i = 0; i < numVoices; ++i)
        {
            synthEngine.addVoice (new VoiceType {});
        }
    }

    void setModulationIndexForModulator (int modulator, double index)
    {
        for (auto i = 0; i < synthEngine.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<VoiceType*> (synthEngine.getVoice (i)))
            {
                voice->getOscillator().setModulationIndex (modulator, index);
            }
        }
    }

    void setRatioForModulator (int modulator, double ratio)
    {
        for (auto i = 0; i < synthEngine.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<VoiceType*> (synthEngine.getVoice (i)))
            {
                voice->getOscillator().setModulationIndex (modulator, ratio);
            }
        }
    }
};