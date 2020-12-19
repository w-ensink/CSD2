
// Written by Wouter Ensink

#pragma once

#include <array>
#include <butterworth/Butterworth.h>
#include <juce_core/juce_core.h>

template <typename T>
inline constexpr T wrap (T dividend, const T divisor) noexcept
{
    while (dividend >= divisor)
        dividend -= divisor;

    return dividend;
}

// should represent one cycle of a waveform
class WaveTable
{
public:
    float getSample (float normalizedPosition)
    {
        auto numSamples = table.size();
        auto index = normalizedPosition * numSamples;
        auto index0 = (int) std::floor (index);
        auto index1 = (int) std::ceil (index);
        auto sample0 = table[index0];
        return 0.0;
    }

private:
    std::vector<float> table;
};


template <typename CarrierType, typename ModulatorType>
struct FmOscillator
{
public:
    FmOscillator() = default;
    ~FmOscillator() = default;

    void setRatio (double newRatio)
    {
        ratio = newRatio;
    }

    void setSampleRate (double newRate)
    {
        sampleRate = newRate;
        carrier.setSampleRate (sampleRate);
        modulator.setSampleRate (sampleRate);
    }

    void setModulationIndex (double index)
    {
        modulationIndex = index;
    }

    void setFrequency (double freq)
    {
        frequency = freq;
        carrier.setFrequency (freq);
        modulator.setFrequency (freq * ratio);
    }

    void advance()
    {
        modulator.advance();
        carrier.setFrequency (frequency + modulator.getSample() * (modulationIndex * frequency * ratio));
        carrier.advance();
        currentSample = carrier.getSample();
    }

    [[nodiscard]] float getSample() const noexcept
    {
        return currentSample;
    }

    auto& getCarrier()
    {
        return carrier;
    }

    auto& getModulator()
    {
        return modulator;
    }

private:
    CarrierType carrier;
    ModulatorType modulator;
    double ratio = 0.5;
    double modulationIndex = 0.5;
    double sampleRate = 0;
    double frequency = 0;
    float currentSample = 0.0f;
};


struct SineWaveOscillator
{
    SineWaveOscillator() = default;
    ~SineWaveOscillator() = default;

    void setSampleRate (double rate) noexcept
    {
        sampleRate = rate;
        recalculateDeltaPhase();
    }

    void setFrequency (double freq) noexcept
    {
        frequency = freq;
        recalculateDeltaPhase();
    }

    void advance() noexcept
    {
        normalizedPhase = wrap (normalizedPhase + deltaPhase, 1.0);
        currentSample = (float) std::sin (normalizedPhase * juce::MathConstants<double>::twoPi);
    }

    [[nodiscard]] float getSample() const noexcept
    {
        return currentSample;
    }

private:
    float currentSample = 1.0;
    double sampleRate = 0;
    double normalizedPhase = 0;
    double frequency = 0;
    double deltaPhase = 0;

    void recalculateDeltaPhase()
    {
        deltaPhase = frequency / sampleRate;
    }
};


// just produces naive square wave
struct SquareWaveOscillator
{
    SquareWaveOscillator() = default;

    void setSampleRate (double rate) noexcept
    {
        sampleRate = rate;
        recalculateDeltaPhase();
    }

    void setFrequency (double freq) noexcept
    {
        frequency = freq;
        recalculateDeltaPhase();
    }

    void advance() noexcept
    {
        normalizedPhase = wrap (normalizedPhase + deltaPhase, 1.0);
        currentSample = normalizedPhase < 0.5 ? 1.0 : -1.0;
    }

    [[nodiscard]] float getSample() const noexcept
    {
        return currentSample;
    }

protected:
    float currentSample = 1.0;
    double sampleRate = 0;
    double normalizedPhase = 0;
    double frequency = 0;
    double deltaPhase = 0;

    void recalculateDeltaPhase()
    {
        deltaPhase = frequency / sampleRate;
    }
};


template <typename OscillatorType, int OversamplingFactor = 8>
struct AntiAliasedOscillator : public OscillatorType
{
public:
    AntiAliasedOscillator() = default;

    void setSampleRate (double rate) noexcept
    {
        OscillatorType::setSampleRate (rate * OversamplingFactor);
        biquadCoefficients.reserve (4);
        auto filterOrder = 8;
        auto validFilter = Butterworth().loPass (rate * OversamplingFactor,
                                                 20'000,
                                                 0,
                                                 filterOrder,
                                                 biquadCoefficients,
                                                 filterGain);
        jassert (validFilter);
        butterworthFilter.resize (4);
    }

    void setFrequency (double frequency) noexcept
    {
        OscillatorType::setFrequency (frequency);
    }

    void advance() noexcept
    {
        for (auto i = 0; i < OversamplingFactor; ++i)
        {
            OscillatorType::advance();
            signalBuffer[i] = OscillatorType::getSample();
        }

        constexpr auto stride = 1;
        butterworthFilter.processBiquad (signalBuffer.data(),
                                         filteredSignalBuffer.data(),
                                         stride,
                                         OversamplingFactor,
                                         biquadCoefficients.data());

        for (auto& sample : filteredSignalBuffer)
            sample *= filterGain;
    }

    [[nodiscard]] float getSample() const noexcept
    {
        return filteredSignalBuffer[0];
    }

private:
    std::array<float, OversamplingFactor> signalBuffer;
    std::array<float, OversamplingFactor> filteredSignalBuffer;
    BiquadChain butterworthFilter;
    std::vector<Biquad> biquadCoefficients;
    double filterGain = 1.0;
};