
#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>


struct PlayHead
{
    double tickTimeMs = 0;
    double timeSinceLastTickMs = 0;
    double blockDurationMs = 0;
    uint64_t startTick = 0;
    std::optional<juce::Range<uint64_t>> loopingRangeTicks = std::nullopt;


    // looping from start to (including) end
    void setLooping (uint64_t start, uint64_t end)
    {
        loopingRangeTicks = { start, end };
        if (! (*loopingRangeTicks).contains (startTick))
        {
            startTick = start;
        }
    }

    [[nodiscard]] bool isLooping() const { return loopingRangeTicks.has_value(); }

    void advanceDeviceBuffer()
    {
        auto timePoint = 0.0;

        if (timeSinceLastTickMs > 0.0)
            timePoint = tickTimeMs - timeSinceLastTickMs;

        while (timePoint < blockDurationMs - tickTimeMs)
            timePoint += tickTimeMs;

        timeSinceLastTickMs = blockDurationMs - timePoint;
    }
};


template <typename Function>
auto forEachTick (const PlayHead& playHead, Function&& function)
{
    auto timeSinceLastTick = playHead.timeSinceLastTickMs;
    auto tickTime = playHead.tickTimeMs;
    auto currentTick = playHead.startTick;
    auto timePoint = 0.0;

    if (timeSinceLastTick > 0.0)
        timePoint = tickTime - timeSinceLastTick;

    while (timePoint < playHead.blockDurationMs)
    {
        function (currentTick, timePoint);
        timePoint += tickTime;
        currentTick += 1;

        if (playHead.isLooping())
            if (currentTick > playHead.loopingRangeTicks->getEnd())
                currentTick = playHead.loopingRangeTicks->getStart();

        timeSinceLastTick = 0.0;
    }
}