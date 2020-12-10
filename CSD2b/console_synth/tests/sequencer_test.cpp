
#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/sequencer.h>

#include <juce_audio_basics/juce_audio_basics.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include <console_synth/sequencer/playhead.h>

auto makeZeroedBufferOfSize (int size)
{
    auto buffer = juce::AudioBuffer<float> (1, size);
    for (auto s = 0; s < size; ++s)
    {
        buffer.getWritePointer (0)[s] = 0.0f;
    }
    return buffer;
}


namespace tst
{
struct PlayHead
{
    void setSessionPositionMs (double positionMs)
    {
        sessionTimeRangeCurrentBlockMs.setStart (positionMs);
        sessionTimeRangeCurrentBlockMs.setEnd (positionMs);
    }


    void updateStreamTimeRange (juce::Range<double> range)
    {
        if (sessionTimeRangeCurrentBlockMs.getLength() == 0)
            sessionTimeRangeCurrentBlockMs.setEnd(sessionTimeRangeCurrentBlockMs.getStart() + range.getLength());
        else
            sessionTimeRangeCurrentBlockMs += range.getLength();
        streamTimeRangeCurrentBlockMs = range;
    }

    juce::Range<double> sessionTimeRangeCurrentBlockMs { 0, 0 };
    juce::Range<double> streamTimeRangeCurrentBlockMs { 0, 0 };
};

struct Event
{
    double sessionTimeStampMs;
};

struct EventHandler
{
    std::vector<Event> events;

    void handleEvents (const PlayHead& playHead, juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& targetBuffer)
    {
        for (auto e : events)
        {
            auto timeStampMs = e.sessionTimeStampMs;

            if (playHead.sessionTimeRangeCurrentBlockMs.contains (timeStampMs))
            {
                // find out the percentile position within the current session range
                auto offsetFromStart = timeStampMs - playHead.sessionTimeRangeCurrentBlockMs.getStart();
                auto normalizedPositionWithinBlock = offsetFromStart / playHead.sessionTimeRangeCurrentBlockMs.getLength();

                auto sampleIndex = std::round (normalizedPositionWithinBlock * audioBuffer.getNumSamples());

                auto message = juce::MidiMessage::noteOn (1, 60, 1.0f);
                targetBuffer.addEvent (message, sampleIndex);
            }
        }
    }
};

}  // namespace tst


struct PlaybackContext
{
    void handleNextBlock (juce::AudioBuffer<float>& audioBuffer, tst::PlayHead& playHead)
    {
        midiBuffer.clear();
        auto blockSize = (double) audioBuffer.getNumSamples();
        CHECK (blockSize == 200);
        auto blockDurationMs = (blockSize / sampleRate) * 1000.0;
        auto streamTimeRange = juce::Range<double> { streamTimeMs, streamTimeMs + blockDurationMs };
        streamTimeMs += blockDurationMs;

        playHead.updateStreamTimeRange (streamTimeRange);

        eventHandler.handleEvents (playHead, audioBuffer, midiBuffer);
    }

    void setSampleRate (double rate)
    {
        sampleRate = rate;
    }

    double streamTimeMs = 0.0;
    double sampleRate = 0.0;

    juce::MidiBuffer midiBuffer {};
    tst::EventHandler eventHandler;
};



// this tests the translation of streaming time to session time
// the PlaybackContext calculates the streaming time based on the block size and sample rate
// and tells the play head about that new streaming time. The play head then translates that to
// session time.
// in this test, the play head starts at session time 100ms. the one event that should be put in a
// midi buffer is scheduled at 1000ms. One buffer is 500ms in length
// this means the event should be fired 400ms into the second buffer, which translates to 80% of the lenght of the buffer
// Since the test uses a sample rate of 400hz and a buffer size of 200 samples,
// the event should be put at 0.8 * 200 == 160 samples
TEST_CASE ("single event in second buffer")
{
    auto blockSize = 200;                                // samples
    auto events = std::vector<tst::Event> { { 1000 } };  // one event at time stamp 1000
    auto sampleRate = 400;                               // samples per second

    auto context = PlaybackContext();
    context.setSampleRate (sampleRate);
    context.eventHandler.events = events;

    REQUIRE (context.midiBuffer.isEmpty());

    auto buffer = makeZeroedBufferOfSize (blockSize);
    auto playHead = tst::PlayHead();
    playHead.setSessionPositionMs (100.0);


    context.handleNextBlock (buffer, playHead);

    CHECK_THAT (playHead.sessionTimeRangeCurrentBlockMs.getStart(), Catch::Matchers::WithinAbs (100, 0.1));
    CHECK_THAT (playHead.sessionTimeRangeCurrentBlockMs.getEnd(), Catch::Matchers::WithinAbs (600, 0.1));

    REQUIRE (context.midiBuffer.isEmpty());

    context.handleNextBlock (buffer, playHead);

    CHECK_THAT (playHead.sessionTimeRangeCurrentBlockMs.getStart(), Catch::Matchers::WithinAbs (600, 0.1));
    CHECK_THAT (playHead.sessionTimeRangeCurrentBlockMs.getEnd(), Catch::Matchers::WithinAbs (1100, 0.1));

    REQUIRE (! context.midiBuffer.isEmpty());
    auto m = context.midiBuffer.findNextSamplePosition(0);

    // the position relative to the audio buffer should be the 160th sample.
    REQUIRE((*m).samplePosition == 160);
}
