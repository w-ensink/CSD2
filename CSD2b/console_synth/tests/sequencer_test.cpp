
#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/sequencer.h>

#include <juce_audio_basics/juce_audio_basics.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include <console_synth/sequencer/play_head.h>

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
    int sessionTimeStampMs;
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

auto convertBpmToTickTimeMs(double bpm, int ticksPerQuarterNote)
{
    auto ticksPerMinute = bpm * ticksPerQuarterNote;
    auto msPerMinute = 60'000;
    return ticksPerMinute / msPerMinute;
}


struct PlayHead
{
    // when processing a buffer, measure the
    int getTick (double timeIntoCurrentRenderPass) const
    {
    }

    auto getStreamTimeRangeForCurrentRenderPass() const
    {
        return streamTimeRangeCurrentBlockMs;
    }

    auto moveToTick (int tick)
    {
        auto positionEditLock = juce::ScopedLock { sessionPositionMutex };
        currentTick = tick;
    }

    bool isPlaying() const { return playing; }

    void setTicksPerQuarterNote (int ticks)
    {
        ticksPerQuarterNote = ticks;
    }

private:
    bool playing = false;
    std::optional<juce::Range<double>> loopingRange = std::nullopt;
    juce::Range<double> streamTimeRangeCurrentBlockMs { 0, 0 };
    juce::CriticalSection sessionPositionMutex;
    int currentTick = 0;
    int ticksPerQuarterNote = 0;
    double tickTime = 0;
};

struct PlaybackContext;

struct TransportControl
{
    explicit TransportControl (PlaybackContext& context) : playbackContext (context) {}

    void startPlayback() { playState = PlayState::playing; }
    void stopPlayback() { playState = PlayState::stopped; }
    void setLoopingRangeMs (double start, double end) {}

    [[nodiscard]] bool isPlaying() const { return playState == PlayState::playing; }

    void moveToTick (int tick);


    void setTicksPerQuarterNote(int ticks) { ticksPerQuarterNote = ticks; }

    // make sure ticks per quarter note is set before calling this one
    void setPlaybackTempoBpm (double bpm) {
        currentBpm = bpm;
       // auto tickTimeMs =
    }

private:
    enum struct PlayState
    {
        playing,
        stopped,
        recording
    };

    PlayState playState = PlayState::stopped;

    PlaybackContext& playbackContext;
    double currentBpm = 0;
    int ticksPerQuarterNote = 0;
};


struct Event
{
    int timeStampTicks;
};

// this should provide playback context with midi from any source (internal and external via midi collector)
// the midi source that plays back the melody should consider the play head,
// the midi source that provides external midi should not care about the play head
struct MidiSource
{
    void fillNextMidiBuffer (const PlayHead& playHead, juce::MidiBuffer& bufferToFill, double numSamples)
    {
        auto bufferStreamTime = playHead.getStreamTimeRangeForCurrentRenderPass().getLength();

        for (auto sample = 0; sample < numSamples; ++sample)
        {
            auto timeIntoBuffer = bufferStreamTime * (sample / numSamples);
            auto tick = playHead.getTick (timeIntoBuffer);

            std::for_each (events.begin(), events.end(), [&] (auto& e) {

            });
        }
    }

    std::vector<tst2::Event> events;
};


// just fills the buffer with noise as a test to make sure buffers get filled;
struct AudioSource
{
    void fillNextBuffer (const PlayHead& playHead, juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer)
    {
        for (auto i = 0; i < audioBuffer.getNumSamples(); ++i)
            audioBuffer.getWritePointer (0)[i] = random.nextFloat();
    }

    juce::Random random;
};


// this should be called from the audio device callback with the current stream time (ms) and the audio buffer to fill
struct PlaybackContext
{
    explicit PlaybackContext (AudioSource& as) : audioSource { as } {}

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
};


void TransportControl::moveToTick (int tick)
{
    playbackContext.playHead.moveToTick (tick);
}

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


TEST_CASE("bpm to ms per tick")
{
    auto bpm = 60;
    auto tpqn = 16;

    auto expected = 0;

    REQUIRE(tst2::convertBpmToTickTimeMs(bpm, tpqn) == expected);
}


// one event at 1000ms session time
// block size 200 samples
// sample rate 400hz
// play head starting from 1000ms
// requires midi message in midi buffer in second device callback
TEST_CASE ("new play head test")
{
    auto blockSize = 200;  // samples

    auto sampleRate = 400.0;  // samples per second
    auto bpm = 60;
    auto ticksPerQuarterNote = 16;  // so 60 * 16 ticks per minute

    // we want an event at around 1000ms: that is 1s or 16 ticks in this example
    auto events = std::vector<tst2::Event> { { 16 } };  // one event at time stamp 1000ms (or 16 ticks with bpm 60 and tpqn 16)

    auto audioSource = tst2::AudioSource {};
    auto playbackContext = tst2::PlaybackContext (audioSource);
    auto transportControl = tst2::TransportControl { playbackContext };


    auto deviceMock = tst2::AudioDeviceMock {
        .sampleRate = sampleRate,
        .blockSize = blockSize,
        .context = playbackContext
    };

    transportControl.setPlaybackTempoBpm (bpm);
    transportControl.moveToTick (100);
    transportControl.startPlayback();


    auto midiSource = tst2::MidiSource();
    midiSource.events = events;

    playbackContext.midiSources.push_back (&midiSource);

    REQUIRE (playbackContext.midiBuffer.isEmpty());

    // after the first device callback, the midi buffer should still be empty
    deviceMock.simulateDeviceCallback();
    REQUIRE (playbackContext.midiBuffer.isEmpty());

    // after the second device callback, the midi buffer should contain one midi message
    deviceMock.simulateDeviceCallback();
    REQUIRE (! playbackContext.midiBuffer.isEmpty());
}