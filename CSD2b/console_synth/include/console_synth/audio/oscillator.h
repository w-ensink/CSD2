
// Written by Wouter Ensink

#pragma once

#include <array>
#include <butterworth/Butterworth.h>
#include <juce_core/juce_core.h>
#include <tuple>

// ===================================================================================================

template <typename T>
inline constexpr T wrap (T dividend, const T divisor) noexcept
{
    while (dividend >= divisor)
        dividend -= divisor;

    return dividend;
}

// ===================================================================================================

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

// ===================================================================================================

template <typename CarrierType, typename ModulatorType>
struct AmOscillator
{
    AmOscillator() = default;
    ~AmOscillator() = default;

    void setRatio (double newRatio)
    {
        ratio = newRatio;
    }

    void setSampleRate (double rate)
    {
        carrier.setSampleRate (rate);
        modulator.setSampleRate (rate);
    }

    void setModulationIndex (double index)
    {
        modulationIndex = index;
    }

private:
    CarrierType carrier;
    ModulatorType modulator;
    double ratio = 1.0;
    double modulationIndex = 1.0;
};

// ===================================================================================================


namespace details
{
template <typename Tuple, typename Functor, size_t Index = 0>
inline constexpr auto forEachTupleItem (Tuple& tuple, Functor&& function) noexcept
{
    function (std::get<Index> (tuple), Index);

    if constexpr (Index + 1 < std::tuple_size_v<Tuple>)
        forEachTupleItem<Tuple, Functor, Index + 1> (tuple, std::forward<Functor> (function));
}
}  // namespace details


// FM oscillator with multiple modulating oscillators
template <typename CarrierType, typename... ModulatorTypes>
struct FmOscillator
{
    void setRatio (int modulator, double ratio)
    {
        ratios[modulator] = ratio;
    }

    void setRatios (const std::array<double, sizeof...(ModulatorTypes)>& newRatios)
    {
        ratios = newRatios;
    }

    void setModulationIndex (int modulator, double modIndex)
    {
        modulationIndices[modulator] = modIndex;
    }

    void setModulationIndices (const std::array<double, sizeof...(ModulatorTypes)>& indices)
    {
        modulationIndices = indices;
    }

    constexpr auto getNumModulators() noexcept
    {
        return sizeof...(ModulatorTypes);
    }

    void advance()
    {
        auto carrierFreq = frequency;

        details::forEachTupleItem (modulators, [this, &carrierFreq] (auto& modulator, auto index) {
            modulator.advance();
            carrierFreq += modulator.getSample() * frequency * ratios[index] * modulationIndices[index];
        });

        carrier.setFrequency (carrierFreq);
        carrier.advance();
        currentSample = carrier.getSample();
    }

    void setFrequency (double freq)
    {
        frequency = freq;

        details::forEachTupleItem (modulators, [this] (auto& modulator, auto index) {
            modulator.setFrequency (ratios[index] * frequency);
        });

        carrier.setFrequency (frequency);
    }

    void setSampleRate (double rate)
    {
        details::forEachTupleItem (modulators, [rate] (auto& modulator, auto index) {
            modulator.setSampleRate (rate);
        });

        carrier.setSampleRate (rate);
    }

    template <size_t ModIndex>
    auto& getModulator()
    {
        return std::get<ModIndex> (modulators);
    }

    auto& getCarrier()
    {
        return carrier;
    }

    [[nodiscard]] float getSample() const noexcept { return currentSample; }

private:
    CarrierType carrier;
    std::tuple<ModulatorTypes...> modulators;
    std::array<double, sizeof...(ModulatorTypes)> ratios;
    std::array<double, sizeof...(ModulatorTypes)> modulationIndices;
    double frequency = 0;
    float currentSample = 0.0;
};

// ===================================================================================================

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

// ===================================================================================================
// just produces naive square wave
struct SquareWaveOscillator
{
    SquareWaveOscillator() = default;
    ~SquareWaveOscillator() = default;

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

// ===================================================================================================

struct SawWaveOscillator
{
    SawWaveOscillator() = default;
    ~SawWaveOscillator() = default;

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
        currentSample = (float) ((normalizedPhase * 2.0) - 1.0);
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

// ===================================================================================================

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

// ===================================================================================================

/*
template <typename OscillatorType>
struct OscillatorController {
    juce::StringArray getAvailableParameters()
    {

    }
};
 */