
// Written by Wouter Ensink

#pragma once

#include "audio_processor_base.h"
#include <console_synth/audio/oscillators.h>

// ===================================================================================================

class GeneralSynthesizerVoice : public juce::SynthesiserSound
{
public:
    bool appliesToChannel (int midiChannel) override { return true; }
    bool appliesToNote (int midiNoteNumber) override { return true; }
};

// ===================================================================================================

template <typename OscillatorType>
class OscillatorSynthesizerVoice : public juce::SynthesiserVoice
{
public:
    OscillatorSynthesizerVoice()
    {
        envelope.setSampleRate (getSampleRate());
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

    void setEnvelope (juce::ADSR::Parameters params)
    {
        envelope.setParameters (params);
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

// ===================================================================================================

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

// ===================================================================================================

// fm synth with anti aliased (8x oversampled) voices with 1 sine carrier and 3 sine modulators
class FmSynthesizer : public SynthesizerBase
{
public:
    using OscType = AntiAliased<FmOsc<SquareOsc<float>, SineOsc<float>, SineOsc<float>, SineOsc<float>>>;
    using VoiceType = OscillatorSynthesizerVoice<OscType>;

    explicit FmSynthesizer (juce::ValueTree parent)
    {
        parent.appendChild (synthState, nullptr);

        for (auto i = 0; i < numVoices.getValue(); ++i)
        {
            auto voice = new VoiceType {};
            voice->getOscillator().setRatios ({ 0.5, 0.25, 2.0 });
            synthEngine.addVoice (voice);
        }

        auto onEnvChange = [this] (auto) { envelopeChanged(); };
        attack.onChange = onEnvChange;
        decay.onChange = onEnvChange;
        sustain.onChange = onEnvChange;
        release.onChange = onEnvChange;
        envelopeChanged();
    }

    void setModulationIndexForModulator (int modulator, double index)
    {
        forEachVoice ([modulator, index] (auto& voice) {
            voice.getOscillator().setModulationIndex (modulator, index);
        });
    }

    void setRatioForModulator (int modulator, double ratio)
    {
        forEachVoice ([modulator, ratio] (auto& voice) {
            voice.getOscillator().setModulationIndex (modulator, ratio);
        });
    }

private:
    juce::ValueTree synthState { IDs::synth };
    Property<int> numVoices { synthState, IDs::numVoices, nullptr, 4 };
    Property<double> modulationIndex { synthState, IDs::modulationIndex, nullptr, 1.0 };
    Property<float> attack { synthState, IDs::attack, nullptr, 0.001 };
    Property<float> decay { synthState, IDs::decay, nullptr, 1.0 };
    Property<float> sustain { synthState, IDs::sustain, nullptr, 0.5 };
    Property<float> release { synthState, IDs::release, nullptr, 1.0 };


    template <typename Functor>
    void forEachVoice (Functor&& function)
    {
        for (auto i = 0; i < synthEngine.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<VoiceType*> (synthEngine.getVoice (i)))
                function (*voice);
    }

    void envelopeChanged()
    {
        auto params = juce::ADSR::Parameters {
            .attack = attack.getValue(),
            .decay = decay.getValue(),
            .sustain = sustain.getValue(),
            .release = release.getValue()
        };

        forEachVoice ([params] (auto& voice) {
            voice.setEnvelope (params);
        });
    }
};