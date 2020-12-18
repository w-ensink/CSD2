
#include <butterworth/Butterworth.h>
#include <catch2/catch_all.hpp>
#include <console_synth/audio/oscillator.h>
#include <console_synth/format.h>
#include <filesystem>

auto getDataDirectoryPath()
{
    return fmt::format ("{}/tests/data", std::filesystem::current_path().parent_path().parent_path().string());
}

TEST_CASE ("oversampling test")
{
    auto sampleRate = 44'100.0;
    auto frequency = 10'000.0;
    auto duration = 200;  // samples

    auto squareOscillator = SquareWaveOscillator();
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

    squareOscillator = SquareWaveOscillator();

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