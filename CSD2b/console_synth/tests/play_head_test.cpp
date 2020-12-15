

#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/play_head.h>


TEST_CASE ("play head start from 0")
{
    const auto tickTimeMs = 1.0;
    const auto audioDeviceCallbackDuration = 100.0;
    const auto startTick = 0;

    auto playHead = PlayHead();
    playHead.setTickTimeMs (tickTimeMs);
    playHead.setDeviceCallbackDurationMs (audioDeviceCallbackDuration);
    playHead.setPositionInTicks (startTick);

    auto count = 0;
    auto firstTick = 5235;
    auto firstTickTimeIntoBuffer = -1.0;

    forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
        ++count;
        if (count == 1)
        {
            firstTick = tick;
            firstTickTimeIntoBuffer = timeIntoBufferMs;
        }
    });

    REQUIRE (count == 100);
    REQUIRE (firstTick == 0);
    REQUIRE_THAT (firstTickTimeIntoBuffer, Catch::Matchers::WithinAbs (0.0, 0.001));
}


TEST_CASE ("play head start from 1")
{
    const auto tickTimeMs = 1.0;
    const auto audioDeviceCallbackDuration = 100.0;
    const auto startTick = 1;

    auto playHead = PlayHead();
    playHead.setTickTimeMs (tickTimeMs);
    playHead.setDeviceCallbackDurationMs (audioDeviceCallbackDuration);
    playHead.setPositionInTicks (startTick);

    auto timeStampsIntoBufferMs = std::vector<double>();
    auto ticks = std::vector<uint64_t>();

    forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
        ticks.push_back (tick);
        timeStampsIntoBufferMs.push_back (timeIntoBufferMs);
    });

    REQUIRE (ticks.size() == 100);
    REQUIRE (ticks[0] == 1);
    REQUIRE (ticks[1] == 2);
    REQUIRE_THAT (timeStampsIntoBufferMs[0], Catch::Matchers::WithinAbs (0.0, 0.001));
}


// in this case, the last handled tick was 0.3ms ago
// since the tick time is 1.0 ms, the next tick (number 50), should be handled
// at time 0.7ms into the current buffer
TEST_CASE ("play head complex timing")
{
    const auto tickTimeMs = 1.0;
    const auto audioDeviceCallbackDuration = 100.0;
    const auto startTick = 50;

    auto playHead = PlayHead();
    playHead.setTickTimeMs (tickTimeMs);
    playHead.setDeviceCallbackDurationMs (audioDeviceCallbackDuration);
    playHead.setPositionInTicks (startTick);
    playHead.setTimeSinceLastTickMs (0.3);

    auto timeStampsIntoBufferMs = std::vector<double>();
    auto ticks = std::vector<uint64_t>();

    forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
        ticks.push_back (tick);
        timeStampsIntoBufferMs.push_back (timeIntoBufferMs);
    });

    CHECK (ticks.size() == 100);
    CHECK (ticks[0] == 50);
    CHECK (ticks[1] == 51);
    CHECK_THAT (timeStampsIntoBufferMs[0], Catch::Matchers::WithinAbs (0.7, 0.001));
}


TEST_CASE ("play head, multiple callbacks")
{
    const auto tickTimeMs = 0.3;
    const auto audioDeviceCallbackDuration = 100.0;
    const auto startTick = 50;

    auto playHead = PlayHead();
    playHead.setTickTimeMs (tickTimeMs);
    playHead.setDeviceCallbackDurationMs (audioDeviceCallbackDuration);
    playHead.setPositionInTicks (startTick);
    playHead.setTimeSinceLastTickMs (0.4);

    auto lastTicksBufferTime = 0.0;

    forEachTick (playHead, [&] (uint64_t, double bufferTimeMs) {
        lastTicksBufferTime = bufferTimeMs;
    });

    // advances the play head one io device callback buffer
    playHead.advanceDeviceBuffer();

    auto timeSinceLastTick = audioDeviceCallbackDuration - lastTicksBufferTime;

    REQUIRE (playHead.getTimeSinceLastTickMs() == timeSinceLastTick);
}


TEST_CASE ("play head basic looping")
{
    const auto tickTimeMs = 1.0;
    const auto audioDeviceCallbackDuration = 100.0;
    const auto startTick = 0;

    auto playHead = PlayHead();
    playHead.setTickTimeMs (tickTimeMs);
    playHead.setDeviceCallbackDurationMs (audioDeviceCallbackDuration);
    playHead.setPositionInTicks (startTick);

    // loop from tick 5 to 49 (including), so 5-50 exclusive
    playHead.setLooping (5, 49);
    REQUIRE (playHead.isLooping());

    auto timeStampsIntoBufferMs = std::vector<double>();
    auto ticks = std::vector<uint64_t>();

    forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
        ticks.push_back (tick);
        timeStampsIntoBufferMs.push_back (timeIntoBufferMs);
    });


    // after 100ms (block duration), we should have had 100 ticks
    REQUIRE (ticks.size() == 100);

    // first tick that should be handled is 5
    REQUIRE (ticks[0] == 5);
    REQUIRE (ticks[1] == 6);

    // after tick 49, it should wrap back to 5...
    REQUIRE (ticks[44] == 49);
    REQUIRE (ticks[45] == 5);

    CHECK(ticks.back() == 14);

    playHead.advanceDeviceBuffer();
    // after 100 ticks it should be at tick 15
    CHECK(playHead.getCurrentTick() == 15);

    playHead.advanceDeviceBuffer();
    // another 100 ticks later it should be at tick 25
    CHECK(playHead.getCurrentTick() == 25);
}


TEST_CASE ("play head realistic scenario")
{
    auto playHead = PlayHead();

    auto sampleRate = 44100.0;
    auto blockSizeSamples = 512;
    auto blockDurationMs = (blockSizeSamples / sampleRate) * 1000.0;

    playHead.setDeviceCallbackDurationMs (blockDurationMs);
    playHead.setPositionInTicks (0);

    auto eventTimeStampTicks = 100;

    auto tempoBpm = 100;
    auto ticksPerQuarterNote = 48;

    auto ticksPerMinute = tempoBpm * ticksPerQuarterNote;
    auto msPerMinute = 60'000.0;
    auto msPerTick = msPerMinute / ticksPerMinute;

    playHead.setTickTimeMs (msPerTick);

    // according to this example, the tick time is 12.5ms
    // the block duration is 11.61ms
    // the 100th tick should come at 1250ms
    // that is the 1250 / 11.61 = 107.67th buffer

    auto eventHandled = false;
    auto eventTimeStampBuffer = -1.0;
    auto bufferCount = 0;

    auto tickTimeStamps = std::vector<double>();
    auto tickNumbers = std::vector<uint64_t>();

    auto lastTick = -1;
    auto prevTimeSinceLastTick = 0.0;

    while (! eventHandled)
    {
        bufferCount++;
        forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
            tickTimeStamps.push_back (timeIntoBufferMs);
            tickNumbers.push_back (tick);

            if (tick == eventTimeStampTicks)
            {
                eventTimeStampBuffer = timeIntoBufferMs;
                eventHandled = true;
            }

            prevTimeSinceLastTick = playHead.getTimeSinceLastTickMs();
            lastTick = tick;
        });

        playHead.advanceDeviceBuffer();
    }


    CHECK (tickTimeStamps.size() == 101);
    CHECK (tickNumbers.size() == 101);
    CHECK (bufferCount == 108);
}