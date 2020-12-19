
// Written by Wouter Ensink

#pragma once

#include <array>
#include <butterworth/Butterworth.h>
#include <juce_core/juce_core.h>
#include <tuple>

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

template <typename T>
inline constexpr T wrap (T dividend, const T divisor) noexcept
{
    while (dividend >= divisor)
        dividend -= divisor;

    return dividend;
}


template <typename FloatType>
struct PrimitiveOscillatorBase
{
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


    [[nodiscard]] FloatType getSample() const noexcept
    {
        return currentSample;
    }

protected:
    FloatType currentSample = 1.0;
    double sampleRate = 0;
    double normalizedPhase = 0;
    double frequency = 0;
    double deltaPhase = 0;

    void recalculateDeltaPhase()
    {
        deltaPhase = frequency / sampleRate;
    }

    void advancePhase() noexcept
    {
        normalizedPhase = wrap (normalizedPhase + deltaPhase, 1.0);
    }
};


// ===================================================================================================
template <typename FloatType>
struct SineOsc : public PrimitiveOscillatorBase<FloatType>
{
    SineOsc() = default;
    ~SineOsc() = default;

    template <typename T>
    using type = SineOsc<T>;


    void advance() noexcept
    {
        Base::advancePhase();
        Base::currentSample = (FloatType) std::sin (Base::normalizedPhase * juce::MathConstants<double>::twoPi);
    }

private:
    using Base = PrimitiveOscillatorBase<FloatType>;
};

// ===================================================================================================
// just produces naive square wave
template <typename FloatType>
struct SquareOsc : public PrimitiveOscillatorBase<FloatType>
{
    SquareOsc() = default;
    ~SquareOsc() = default;

    template <typename T>
    using type = SquareOsc<T>;

    void advance() noexcept
    {
        Base::advancePhase();
        Base::currentSample = Base::normalizedPhase < 0.5 ? 1.0 : -1.0;
    }

private:
    using Base = PrimitiveOscillatorBase<FloatType>;
};
// ===================================================================================================

template <typename FloatType>
struct SawOsc : public PrimitiveOscillatorBase<FloatType>
{
    SawOsc() = default;
    ~SawOsc() = default;

    template <typename T>
    using type = SawOsc<T>;

    void advance() noexcept
    {
        Base::advancePhase();
        Base::currentSample = (FloatType) ((Base::normalizedPhase * 2.0) - 1.0);
    }

private:
    using Base = PrimitiveOscillatorBase<FloatType>;
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


// ===================================================================================================

template <typename FloatType,
          template <typename> typename CarrierType,
          template <typename> typename... ModulatorTypes>
struct ModulationOscillatorBase
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

    auto& getCarrier() { return carrier; }

    [[nodiscard]] FloatType getSample() const noexcept { return currentSample; }

protected:
    CarrierType<FloatType> carrier;
    std::tuple<ModulatorTypes<FloatType>...> modulators;
    std::array<double, sizeof...(ModulatorTypes)> ratios;
    std::array<double, sizeof...(ModulatorTypes)> modulationIndices;
    double frequency = 0;
    float currentSample = 0.0;
};

// ===================================================================================================

template <typename FloatType,
          template <typename> typename CarrierType,
          template <typename> typename... ModulatorTypes>
struct FmOsc : public ModulationOscillatorBase<FloatType, CarrierType, ModulatorTypes...>
{
    template <typename T>
    using type = FmOsc<T, CarrierType, ModulatorTypes...>;

    void advance() noexcept
    {
        auto carrierFreq = Base::frequency;

        details::forEachTupleItem (Base::modulators, [this, &carrierFreq] (auto& modulator, auto index) {
            modulator.advance();
            carrierFreq += modulator.getSample() * Base::frequency * Base::ratios[index] * Base::modulationIndices[index];
        });

        Base::carrier.setFrequency (carrierFreq);
        Base::carrier.advance();
        Base::currentSample = Base::carrier.getSample();
    }

private:
    using Base = ModulationOscillatorBase<FloatType, CarrierType, ModulatorTypes...>;
};

// ===================================================================================================
// with ring modulation, the modulation indices are used for scaling the amplitude of the modulator
template <typename FloatType,
          template <typename> typename CarrierType,
          template <typename> typename... ModulatorTypes>
struct RmOsc : public ModulationOscillatorBase<FloatType, CarrierType, ModulatorTypes...>
{
    template <typename T>
    using type = RmOsc<T, CarrierType, ModulatorTypes...>;

    void advance() noexcept
    {
        Base::carrier.advance();
        auto sample = Base::carrier.getSample();

        details::forEachTupleItem (Base::modulators, [this, &sample] (auto& modulator, auto index) {
            modulator.advance();
            sample *= (modulator.getSample() * Base::modulationIndices[index]);
        });

        Base::currentSample = sample;
    }

private:
    using Base = ModulationOscillatorBase<FloatType, CarrierType, ModulatorTypes...>;
};

// ===================================================================================================

template <typename FloatType,
          template <typename> typename OscillatorType,
          int OversamplingFactor = 8>
struct AAOsc : public OscillatorType<FloatType>
{
public:
    AAOsc() = default;
    ~AAOsc() = default;

    template <typename T>
    using type = AAOsc<T, OscillatorType, OversamplingFactor>;


    void setSampleRate (double rate) noexcept
    {
        Base::setSampleRate (rate * OversamplingFactor);
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
        Base::setFrequency (frequency);
    }

    void advance() noexcept
    {
        for (auto i = 0; i < OversamplingFactor; ++i)
        {
            Base::advance();
            signalBuffer[i] = Base::getSample();
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

    using Base = OscillatorType<FloatType>;
};

// ===================================================================================================
/*
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
*/
// ===================================================================================================

/*
template <typename OscillatorType>
struct OscillatorController {
    juce::StringArray getAvailableParameters()
    {

    }
};
 */