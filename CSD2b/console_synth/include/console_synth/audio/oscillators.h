
// Written by Wouter Ensink

#pragma once

#include <array>
#include <butterworth/Butterworth.h>
#include <juce_core/juce_core.h>
#include <tuple>

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


template <typename T>
inline constexpr T wrap (T dividend, const T divisor) noexcept
{
    while (dividend >= divisor)
        dividend -= divisor;

    return dividend;
}

}  // namespace details


// ===================================================================================================


template <typename FloatType>
struct PrimitiveOscillatorBase
{
    using float_type = FloatType;

    static_assert (std::is_floating_point_v<float_type>, "oscillator requires a floating point type");

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
        normalizedPhase = details::wrap (normalizedPhase + deltaPhase, 1.0);
    }

    PrimitiveOscillatorBase() = default;
};


// ===================================================================================================
template <typename FloatType>
struct SineOsc : public PrimitiveOscillatorBase<FloatType>
{
    SineOsc() = default;
    ~SineOsc() = default;

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
struct TriangleOsc : public PrimitiveOscillatorBase<FloatType>
{
    TriangleOsc() = default;
    ~TriangleOsc() = default;

    void advance() noexcept
    {
        Base::advancePhase();

        if (Base::normalizedPhase < 0.5)
            Base::currentSample = Base::normalizedPhase * 4.0 - 1.0;
        else
            Base::currentSample = (1.0 - Base::normalizedPhase) * 4.0 - 1.0;
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

    void advance() noexcept
    {
        Base::advancePhase();
        Base::currentSample = (FloatType) ((Base::normalizedPhase * 2.0) - 1.0);
    }

private:
    using Base = PrimitiveOscillatorBase<FloatType>;
};

// ===================================================================================================

template <typename FloatType>
struct NoiseOsc
{
    using float_type = FloatType;

    static_assert (std::is_floating_point_v<float_type>, "oscillator requires a floating point type");

    void setFrequency (double) {}
    void setSampleRate (double) {}

    FloatType getSample() const noexcept
    {
        return currentSample;
    }

    void advance() noexcept
    {
        currentSample = (FloatType) random.nextDouble();
    }

private:
    juce::Random random;
    FloatType currentSample;
};

// ===================================================================================================

template <typename CarrierType, typename... ModulatorTypes>
struct ModulationOscillatorBase
{
    static_assert ((std::is_same_v<typename CarrierType::float_type, typename ModulatorTypes::float_type> && ...),
                   "all oscillator float types must match");

    using float_type = typename CarrierType::float_type;

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

    [[nodiscard]] float_type getSample() const noexcept { return currentSample; }

protected:
    CarrierType carrier;
    std::tuple<ModulatorTypes...> modulators;
    std::array<double, sizeof...(ModulatorTypes)> ratios;
    std::array<double, sizeof...(ModulatorTypes)> modulationIndices;
    double frequency = 0;
    float_type currentSample = 0.0;

    ModulationOscillatorBase() = default;
};

// ===================================================================================================

template <typename CarrierType, typename... ModulatorTypes>
struct FmOsc : public ModulationOscillatorBase<CarrierType, ModulatorTypes...>
{
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
    using Base = ModulationOscillatorBase<CarrierType, ModulatorTypes...>;
};

// ===================================================================================================
// with ring modulation, the modulation indices are used for scaling the amplitude of the modulator
template <typename CarrierType, typename... ModulatorTypes>
struct RmOsc : public ModulationOscillatorBase<CarrierType, ModulatorTypes...>
{
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
    using Base = ModulationOscillatorBase<CarrierType, ModulatorTypes...>;
};

// ===================================================================================================

template <typename CarrierType, typename... ModulatorTypes>
struct AmOsc : public ModulationOscillatorBase<CarrierType, ModulatorTypes...>
{
    void advance() noexcept
    {
        Base::carrier.advance();
        auto sample = Base::carrier.getSample();

        details::forEachTupleItem (Base::modulators, [this, &sample] (auto& modulator, auto index) {
            modulator.advance();
            sample *= (std::abs (modulator.getSample()) * Base::modulationIndices[index]);
        });

        Base::currentSample = sample;
    }

private:
    using Base = ModulationOscillatorBase<CarrierType, ModulatorTypes...>;
};

// ===================================================================================================

template <typename OscillatorType, int OversamplingFactor = 8>
struct AntiAliased : public OscillatorType
{
public:
    AntiAliased() = default;
    ~AntiAliased() = default;

    using float_type = typename OscillatorType::float_type;


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
    std::array<float_type, OversamplingFactor> signalBuffer;
    std::array<float_type, OversamplingFactor> filteredSignalBuffer;
    BiquadChain butterworthFilter;
    std::vector<Biquad> biquadCoefficients;
    double filterGain = 1.0;

    using Base = OscillatorType;
};
