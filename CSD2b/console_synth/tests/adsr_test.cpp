
// Written by Wouter Ensink

#include <catch2/catch_all.hpp>
#include <console_synth/audio/envelope.h>

TEST_CASE ("adsr simple 1ms decay, 0.5 sustain, without attack and release")
{
    auto sampleRate = 44100;
    auto params = juce::ADSR::Parameters { 0, 0.001, 0.5, 0.0 };

    auto env = ADSR {};
    env.setSampleRate (sampleRate);
    env.setParameters (params);

    CHECK (! env.isActive());

    env.getNextSample();
    CHECK (! env.isActive());

    env.noteOn();
    CHECK (env.isActive());

    // 1 ms of decay without attack should be at 0.5 after 44.1 samples
    for (auto s = 0; s < 44; ++s)
        env.getNextSample();

    CHECK (env.isActive());
    CHECK_THAT (env.getNextSample(), Catch::Matchers::WithinAbsMatcher (0.5, 0.0001));

    env.noteOff();
    CHECK (! env.isActive());
}


TEST_CASE ("test release time and callback functionality")
{
    auto sampleRate = 44100;
    auto params = juce::ADSR::Parameters { 0, 0.001, 0.5, 0.001 };

    auto env = ADSR {};
    env.setSampleRate (sampleRate);
    env.setParameters (params);

    auto releaseFinishedCalled = false;

    env.setReleaseFinishedCallback ([&] { releaseFinishedCalled = true; });

    CHECK (! env.isActive());

    env.noteOn();
    CHECK (env.isActive());
    CHECK (! releaseFinishedCalled);

    for (auto i = 0; i < 100; ++i)
        env.getNextSample();

    CHECK (env.isActive());
    CHECK (! releaseFinishedCalled);

    env.noteOff();

    CHECK (env.isActive());
    CHECK (! releaseFinishedCalled);

    // 0.001 seconds -> 44.1 samples later release finished called
    for (auto i = 0; i < 100; ++i)
        env.getNextSample();

    CHECK (! env.isActive());
    CHECK (releaseFinishedCalled);
}