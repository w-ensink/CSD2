
// Written by Wouter Ensink

#include <juce_audio_processors/juce_audio_processors.h>

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