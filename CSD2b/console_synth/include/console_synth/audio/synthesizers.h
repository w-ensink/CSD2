
// Written by Wouter Ensink

#pragma once

#include "audio_processor_base.h"
#include <console_synth/audio/envelope.h>
#include <console_synth/audio/oscillators.h>
#include <console_synth/identifiers.h>
#include <console_synth/utility/format.h>
#include <console_synth/utility/property.h>

// ===================================================================================================

class GeneralSynthesizerSound : public juce::SynthesiserSound
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
        envelope.setReleaseFinishedCallback ([this] { clearCurrentNote(); });
        oscillator.setSampleRate (getSampleRate());
    }

    ~OscillatorSynthesizerVoice() override = default;

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        for (auto sample = startSample; sample < numSamples + startSample; ++sample)
        {
            auto value = getNextSample();

            for (auto channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample (channel, sample, value);
        }
    }

    void stopNote (float velocity, bool allowTailOff) override
    {
        envelope.noteOff();
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        oscillator.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
        envelope.reset();
        envelope.noteOn();
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<GeneralSynthesizerSound*> (sound) != nullptr;
    }

    void controllerMoved (int controllerNumber, int newControllerValue) override {}
    void pitchWheelMoved (int newPitchWheelValue) override {}

    auto& getOscillator()
    {
        return oscillator;
    }

    void setEnvelope (juce::ADSR::Parameters params)
    {
        envelope.setParameters (params);
    }

private:
    OscillatorType oscillator;
    ADSR envelope;


    double getNextSample()
    {
        oscillator.advance();
        return oscillator.getSample() * envelope.getNextSample();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorSynthesizerVoice);
};

// ===================================================================================================

class SynthesizerBase : public AudioProcessorBase
{
public:
    SynthesizerBase()
    {
        synthEngine.addSound (new GeneralSynthesizerSound());
    }

    ~SynthesizerBase() override = default;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthesizerBase);
};

// ===================================================================================================

template <typename OscType>
class ModulationSynthesizer : public SynthesizerBase
{
public:
    using VoiceType = OscillatorSynthesizerVoice<OscType>;

    explicit ModulationSynthesizer (juce::ValueTree parent)
    {
        parent.appendChild (synthState, nullptr);

        auto modIndices = std::array<double, OscType::getNumModulators()> {};

        for (auto& index : modIndices)
            index = 1.0;

        for (auto i = 0; i < numVoices.getValue(); ++i)
        {
            auto voice = new VoiceType {};
            voice->getOscillator().setModulationIndices (modIndices);
            synthEngine.addVoice (voice);
        }

        ratiosChanged();
        ratios.onChange = [this] (auto) { ratiosChanged(); };

        auto onEnvChange = [this] (auto) { envelopeChanged(); };
        attack.onChange = onEnvChange;
        decay.onChange = onEnvChange;
        sustain.onChange = onEnvChange;
        release.onChange = onEnvChange;
        envelopeChanged();
    }


    ~ModulationSynthesizer() override = default;

private:
    juce::ValueTree synthState { IDs::synth };
    Property<int> numVoices { synthState, IDs::numVoices, 4 };
    Property<float> attack { synthState, IDs::attack, 0.001 };
    Property<float> decay { synthState, IDs::decay, 0.1 };
    Property<float> sustain { synthState, IDs::sustain, 0.5 };
    Property<float> release { synthState, IDs::release, 0.1 };
    ArrayProperty ratios { synthState, IDs::ratios, { 0.125, 0.25, 0.5 } };


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


    void ratiosChanged()
    {
        auto newRatios = std::array<double, OscType::getNumModulators()> {};
        auto ratioValues = ratios.getValue();

        for (auto i = 0; i < OscType::getNumModulators(); ++i)
            newRatios[i] = ratioValues[i];

        forEachVoice ([&newRatios] (auto& voice) {
            voice.getOscillator().setRatios (newRatios);
        });
    }


    template <typename Functor>
    void forEachVoice (Functor&& function)
    {
        for (auto i = 0; i < synthEngine.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<VoiceType*> (synthEngine.getVoice (i)))
                function (*voice);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulationSynthesizer);
};

// ===================================================================================================

using FmSynthesizer = ModulationSynthesizer<
    AntiAliased<
        FmOsc<
            SquareOsc<float>,
            SineOsc<float>,
            SineOsc<float>,
            SineOsc<float>>>>;

// ===================================================================================================

using RmSynthesizer = ModulationSynthesizer<
    AntiAliased<
        RmOsc<
            SineOsc<float>,
            TriangleOsc<float>,
            SawOsc<float>>>>;