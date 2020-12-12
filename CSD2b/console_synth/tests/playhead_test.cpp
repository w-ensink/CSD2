

#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/playhead.h>


TEST_CASE ("play head start from 0")
{
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .blockDurationMs = 100.0,
        .tickBeforeCallback = 0,
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


TEST_CASE("play head start from 1")
{
    auto playHead = PlayHead {
        .tickTimeMs = 1.0,
        .blockDurationMs = 100.0,
        .tickBeforeCallback = 1,
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
    REQUIRE (firstTick == 1);
    REQUIRE_THAT (firstTickTimeIntoBuffer, Catch::Matchers::WithinAbs (0.0, 0.001));
}