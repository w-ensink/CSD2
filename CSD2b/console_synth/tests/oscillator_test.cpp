
// Written by Wouter Ensink

#include <butterworth/Butterworth.h>
#include <catch2/catch_all.hpp>
#include <console_synth/audio/oscillators.h>
#include <console_synth/utility/format.h>
#include <filesystem>

auto getDataDirectoryPath()
{
    return fmt::format ("{}/tests/data", std::filesystem::current_path().parent_path().parent_path().string());
}

TEST_CASE ("oversampling test")
{
    auto sampleRate = 44'100.0;
    auto frequency = 10000.0;
    auto duration = 100;  // samples

    auto squareOscillator = SquareOsc<float>();
    squareOscillator.setSampleRate (sampleRate);
    squareOscillator.setFrequency (frequency);

    // one second of mono audio
    auto buffer = std::vector<float> (duration);

    // test executables are run from root/bin/tests
    auto dataDirPath = getDataDirectoryPath();

    auto naiveSquareFile = juce::File { fmt::format ("{}/square_osc_naive.csv", dataDirPath) };
    naiveSquareFile.deleteFile();
    auto outputStream = juce::FileOutputStream { naiveSquareFile };

    REQUIRE (outputStream.openedOk());

    for (auto& sample : buffer)
    {
        squareOscillator.advance();
        sample = squareOscillator.getSample();
    }

    for (auto& sample : buffer)
    {
        outputStream.writeText (fmt::format ("{}\n", sample), false, false, nullptr);
    }

    // now filter with butterworth filter on oversampled signal
    // =====================================================================================================

    auto biQuadFilters = std::vector<Biquad> (8);
    auto butterworth = Butterworth();
    auto gain = 1.0;
    auto succes = butterworth.loPass (sampleRate * 4, sampleRate / 2, 0, 4, biQuadFilters, gain);
    REQUIRE (succes);

    fmt::print ("gain: {}", gain);

    // oversample 4 times
    auto overSampledBuffer = std::vector<float> (duration * 4);
    auto filteredBuffer = std::vector<float> (duration * 4);

    squareOscillator = SquareOsc<float>();

    squareOscillator.setFrequency (frequency);
    squareOscillator.setSampleRate (sampleRate * 4);

    for (auto& sample : overSampledBuffer)
    {
        squareOscillator.advance();
        sample = squareOscillator.getSample();
    }

    // now apply filter
    auto chain = BiquadChain (biQuadFilters.size());

    chain.processBiquad (overSampledBuffer.data(),
                         filteredBuffer.data(),
                         1,
                         (int) duration * 4,
                         biQuadFilters.data());

    // now downsample
    for (auto i = 0; i < duration; ++i)
    {
        buffer[i] = filteredBuffer[i * 4] * gain;
    }

    auto antiAliasedSquareFile = juce::File { fmt::format ("{}/square_osc_anti_aliased.csv", dataDirPath) };
    antiAliasedSquareFile.deleteFile();
    auto antiAliasedOutputStream = juce::FileOutputStream { antiAliasedSquareFile };


    REQUIRE (antiAliasedOutputStream.openedOk());

    for (auto& sample : buffer)
    {
        antiAliasedOutputStream.writeText (fmt::format ("{}\n", sample), false, false, nullptr);
    }
}


TEST_CASE ("oversampling oscillator template class test")
{
    // anti aliased oscillator with 4x oversampling
    auto overSamplingSquareOscillator = AntiAliased<SquareOsc<float>, 4>();

    auto duration = 100;
    auto buffer = std::vector<float> (duration);
    auto sampleRate = 44100.0;
    auto frequency = 10000;

    overSamplingSquareOscillator.setSampleRate (sampleRate);
    overSamplingSquareOscillator.setFrequency (frequency);

    for (auto& sample : buffer)
    {
        overSamplingSquareOscillator.advance();
        sample = overSamplingSquareOscillator.getSample();
    }

    auto outputFile = juce::File { fmt::format ("{}/square_templated_aa.csv", getDataDirectoryPath()) };
    outputFile.deleteFile();

    auto outputStream = juce::FileOutputStream { outputFile };

    for (auto sample : buffer)
    {
        outputStream.writeText (fmt::format ("{}\n", sample), false, false, nullptr);
    }
}


struct RenderSpec
{
    int numSamples;
    double sampleRate;
    double frequency;
};


template <typename OscillatorType>
void makeOscillatorRender (OscillatorType& oscillator, const juce::File& outputFile, const RenderSpec& spec)
{
    outputFile.deleteFile();
    auto outputStream = juce::FileOutputStream { outputFile };

    oscillator.setSampleRate (spec.sampleRate);
    oscillator.setFrequency (spec.frequency);

    auto buffer = std::vector<float> (spec.numSamples);

    for (auto& sample : buffer)
    {
        oscillator.advance();
        sample = oscillator.getSample();
    }

    for (auto sample : buffer)
    {
        outputStream.writeText (fmt::format ("{}\n", sample), false, false, nullptr);
    }
}


TEST_CASE ("fm render test")
{
    auto outputFile = juce::File { fmt::format ("{}/fm_dual_sine_render.csv", getDataDirectoryPath()) };
    auto fmOscillator = FmOsc<SineOsc<float>, SineOsc<float>>();
    fmOscillator.setRatios ({ 2.0 });
    fmOscillator.setModulationIndices ({ 1 });

    auto renderSpec = RenderSpec {
        .numSamples = 1000,
        .sampleRate = 44100.0,
        .frequency = 500
    };

    makeOscillatorRender (fmOscillator, outputFile, renderSpec);
}

TEST_CASE ("multi fm render test")
{
    auto outputFile = juce::File { fmt::format ("{}/fm_quad_sine_render.csv", getDataDirectoryPath()) };
    auto oscillator = FmOsc<SineOsc<float>, SineOsc<float>, SineOsc<float>, SineOsc<float>>();

    oscillator.setRatios ({ 0.5, 2.0, 4.0 });
    oscillator.setModulationIndices ({ 1.0, 1.0, 1.0 });

    auto renderSpec = RenderSpec {
        .numSamples = 1000,
        .sampleRate = 44100.0,
        .frequency = 500
    };

    makeOscillatorRender (oscillator, outputFile, renderSpec);
}
