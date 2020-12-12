
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
            sessionTimeRangeCurrentBlockMs.setEnd (sessionTimeRangeCurrentBlockMs.getStart() + range.getLength());
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
    auto m = context.midiBuffer.findNextSamplePosition (0);

    // the position relative to the audio buffer should be the 160th sample.
    REQUIRE ((*m).samplePosition == 160);
}


namespace tst2
{
struct PlayHead
{
    // when processing a buffer, measure the
    double getSessionTime (double timeIntoCurrentRenderPass)
    {
    }

    auto getStreamTimeRangeForCurrentRenderPass() const
    {
        return streamTimeRangeCurrentBlockMs;
    }

    auto moveToSessionTime (double sessionTime)
    {
        auto positionEditLock = juce::ScopedLock { sessionPositionMutex };
        currentSessionTime = sessionTime;
    }

    bool isPlaying() const { return playing; }

private:
    bool playing;
    std::optional<juce::Range<double>> loopingRange = std::nullopt;
    juce::Range<double> streamTimeRangeCurrentBlockMs { 0, 0 };
    juce::CriticalSection sessionPositionMutex;
    double currentSessionTime = 0;
};


struct TransportControl
{
    void startPlayback() { playState = PlayState::playing; }
    void stopPlayback() { playState = PlayState::stopped; }
    void setLoopingRangeMs (double start, double end) {}

    [[nodiscard]] bool isPlaying() const { return playState == PlayState::playing; }

private:
    enum struct PlayState
    {
        playing,
        stopped,
        recording
    };

    PlayState playState = PlayState::stopped;

    double position = 0;
};


// this should provide playback context with midi from any source (internal and external via midi collector)
// the midi source that plays back the melody should consider the play head,
// the midi source that provides external midi should not care about the play head
struct MidiSource
{
    void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& bufferToFill, double numSamples)
    {
    }

    std::vector<tst::Event> events;
};


struct AudioSource
{
    void fillNextBuffer (const PlayHead& playHead, juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer)
    {
    }
};


// this should be called from the audio device callback with the current stream time (ms) and the audio buffer to fill
struct PlaybackContext
{
    PlaybackContext (TransportControl& tc, AudioSource& as) : audioSource { as }, transportControl { tc } {}

    void prepare (double rate)
    {
        sampleRate = rate;
    }

    void fillNextAudioBlock (juce::Range<double> deviceStreamTimeRange, juce::AudioBuffer<float>& bufferToFill)
    {
        midiBuffer.clear();

        std::for_each (midiSources.begin(), midiSources.end(), [&] (MidiSource* midiSource) {
            midiSource->fillNextMidiBuffer (playHead, midiBuffer, bufferToFill.getNumSamples());
        });

        audioSource.fillNextBuffer (playHead, bufferToFill, midiBuffer);
    }

    double sampleRate = 0;
    juce::MidiBuffer midiBuffer;
    std::vector<MidiSource*> midiSources;
    AudioSource& audioSource;
    PlayHead playHead;
    TransportControl& transportControl;
};


struct AudioDeviceMock
{
    double sampleRate;
    int blockSize;
    PlaybackContext& context;
    juce::Range<double> currentStreamTimeRange { 0, 0 };

    void init()
    {
        currentStreamTimeRange = { 2000.0, 2000.0 + (blockSize / sampleRate) * 1000 };
        context.prepare (sampleRate);
    }

    void simulateDeviceCallback()
    {
        auto block = makeZeroedBufferOfSize (blockSize);
        context.fillNextAudioBlock (currentStreamTimeRange, block);
        currentStreamTimeRange += (blockSize / sampleRate) * 1000;
    }
};


}  // namespace tst2


TEST_CASE ("new play head test")
{
    auto blockSize = 200;                                // samples
    auto events = std::vector<tst::Event> { { 1000 } };  // one event at time stamp 1000
    auto sampleRate = 400.0;                             // samples per second

    auto transportControl = tst2::TransportControl {};
    auto audioSource = tst2::AudioSource {};
    auto context = tst2::PlaybackContext (transportControl, audioSource);

    auto deviceMock = tst2::AudioDeviceMock {
        .sampleRate = sampleRate,
        .blockSize = blockSize,
        .context = context
    };

    auto midiSource = tst2::MidiSource();
    midiSource.events.push_back (tst::Event { 1000 });

    context.midiSources.push_back (&midiSource);

    REQUIRE (context.midiBuffer.isEmpty());


    REQUIRE (context.midiBuffer.isEmpty());
}