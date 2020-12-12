
#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>


struct PlayHead
{
    double tickTimeMs = 0;
    double timeSinceLastTickMs = 0;
    double blockDurationMs = 0;
    uint64_t tickBeforeCallback = 0;
    std::optional<juce::Range<uint64_t>> loopingRangeTicks = std::nullopt;
};


template <typename Function>
auto forEachTick (const PlayHead& playHead, Function&& function)
{
    auto timeSinceLastTick = playHead.timeSinceLastTickMs;
    auto tickTime = playHead.tickTimeMs;
    auto timePoint = 0.0;
    auto currentTick = playHead.tickBeforeCallback;

    while (timePoint < playHead.blockDurationMs)
    {
        auto timeToNextTick = tickTime - timeSinceLastTick;
        function (currentTick, timePoint);
        timePoint += timeToNextTick;
    }


}