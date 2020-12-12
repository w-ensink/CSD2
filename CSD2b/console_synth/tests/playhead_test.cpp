

#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/playhead.h>


TEST_CASE ("play head start from 0")
{
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .blockDurationMs = 100.0,
        .startTick = 0,
    };

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
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .blockDurationMs = 100.0,
        .startTick = 1,
    };

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
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .timeSinceLastTickMs = 0.3,
        .blockDurationMs = 100.0,
        .startTick = 50,
    };

    auto timeStampsIntoBufferMs = std::vector<double>();
    auto ticks = std::vector<uint64_t>();

    forEachTick (playHead, [&] (uint64_t tick, double timeIntoBufferMs) {
        ticks.push_back (tick);
        timeStampsIntoBufferMs.push_back (timeIntoBufferMs);
    });

    REQUIRE (ticks.size() == 100);
    REQUIRE (ticks[0] == 50);
    REQUIRE (ticks[1] == 51);
    REQUIRE_THAT (timeStampsIntoBufferMs[0], Catch::Matchers::WithinAbs (0.7, 0.001));
}


TEST_CASE ("play head, multiple callbacks")
{
    auto playHead = PlayHead {
        .tickTimeMs = 0.3,
        .timeSinceLastTickMs = 0.4,
        .blockDurationMs = 100.0,
        .startTick = 50,
    };

    auto lastTicksBufferTime = 0.0;

    forEachTick (playHead, [&] (uint64_t, double bufferTimeMs) {
        lastTicksBufferTime = bufferTimeMs;
    });

    // advances the play head one io device callback buffer
    playHead.advanceDeviceBuffer();

    auto timeSinceLastTick = playHead.blockDurationMs - lastTicksBufferTime;

    REQUIRE (playHead.timeSinceLastTickMs == timeSinceLastTick);
}


TEST_CASE ("play head basic looping")
{
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .timeSinceLastTickMs = 0.0,
        .blockDurationMs = 100.0,
        .startTick = 0,
    };

    // loop from tick 5 to 55 (including), so 5-56 exclusive
    playHead.setLooping (5, 55);
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

    // after tick 55, it should wrap back to 5...
    // so tick
    REQUIRE (ticks[50] == 55);
    REQUIRE (ticks[51] == 5);
}