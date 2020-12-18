
// Written by Wouter Ensink

#pragma once

#include <butterworth/Butterworth.h>
#include <array>

template <typename T>
inline constexpr T wrap (T dividend, const T divisor) noexcept
{
    while (dividend >= divisor)
        dividend -= divisor;

    return dividend;
}

// a RenderPass is a unique token for a render cycle, it is used to
// tell an oscillator which cycle it's in, this is essential for knowing if it needs to render
// if the render pass the oscillator has saved is not the same as the render pass it's provided,
// the oscillator needs to render a new sample, this system makes FM with feedback possible (I hope)
struct RenderPass
{
public:
    RenderPass() : uniquePassId (globalPassId)
    {
        globalPassId += 1;
    }

    RenderPass (const RenderPass& other) = default;

    RenderPass& operator= (const RenderPass& other) = default;

    [[nodiscard]] bool operator== (const RenderPass& other) const noexcept
    {
        return uniquePassId == other.uniquePassId;
    }

private:
    inline static uint64_t globalPassId;
    uint64_t uniquePassId;
};


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


template <typename SampleType>
class Oscillator
{
public:
    SampleType getSample (const RenderPass& renderPass) const
    {
        if (renderPass == lastProcessedRenderPass)
            return cachedSample;

        cachedSample = renderNextSample();
        return cachedSample;
    }

    void setFrequency (double newFrequency)
    {
    }

private:
    RenderPass lastProcessedRenderPass;
    SampleType cachedSample;


    // renders next sample and advances the oscillator
    SampleType renderNextSample() noexcept
    {
    }
};


class FmRenderer
{
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


template <typename OscillatorType, int OversamplingIndex>
struct AntiAliasedOscillator : public OscillatorType
{
public:
    AntiAliasedOscillator() = default;

    void setSampleRate (double rate) noexcept
    {
        OscillatorType::setSampleRate (rate * OversamplingIndex);
        biquadCoefficients.reserve (4);
        auto nyquist = rate / 2.0;
        auto filterOrder = 8;
        auto validFilter = Butterworth().loPass (rate * OversamplingIndex,
                                                 nyquist,
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
        for (auto i = 0; i < OversamplingIndex; ++i)
        {
            OscillatorType::advance();
            signalBuffer[i] = OscillatorType::getSample();
        }

        butterworthFilter.processBiquad (signalBuffer.data(),
                                         filteredSignalBuffer.data(),
                                         1,
                                         OversamplingIndex,
                                         biquadCoefficients.data());

        for (auto& sample : filteredSignalBuffer)
            sample *= filterGain;
    }

    [[nodiscard]] float getSample() const noexcept
    {
        return filteredSignalBuffer[0];
    }

private:
    std::array<float, OversamplingIndex> signalBuffer;
    std::array<float, OversamplingIndex> filteredSignalBuffer;
    BiquadChain butterworthFilter;
    std::vector<Biquad> biquadCoefficients;
    double filterGain = 1.0;
};