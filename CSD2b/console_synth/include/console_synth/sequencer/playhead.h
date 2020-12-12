
#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>


struct PlayHead
{
private:
    double tickTimeMs = 0;
    double blockDurationMs = 0;
    uint64_t currentTick = 0;
    double timeSinceLastTickMs = 0;
    std::optional<juce::Range<uint64_t>> loopingRangeTicks = std::nullopt;
    juce::CriticalSection mutex;

public:
    PlayHead() = default;

    void setTickTimeMs (double tickTime)
    {
        tickTimeMs = tickTime;
    }

    void setDeviceCallbackDurationMs (double duration)
    {
        blockDurationMs = duration;
    }

    void setPositionInTicks (uint64_t position)
    {
        currentTick = position;
    }

    void setTimeSinceLastTickMs (double time)
    {
        timeSinceLastTickMs = time;
    }


    // returns the time that was left into the last processed buffer
    // after the last time point a tick was scheduled in that buffer
    // e.g.: the last tick in the previous device callback buffer
    // was at 9ms into a 10ms buffer, then this should return 1.
    [[nodiscard]] double getTimeSinceLastTickMs() const
    {
        return timeSinceLastTickMs;
    }

    // looping from start to (including) end
    void setLooping (uint64_t start, uint64_t end)
    {
        loopingRangeTicks = { start, end };

        if (! (*loopingRangeTicks).contains (currentTick))
        {
            currentTick = start;
        }
    }

    // returns the start of the loop in ticks. if not looping, it returns a nullopt
    [[nodiscard]] std::optional<uint64_t> getLoopingStart() const
    {
        if (isLooping())
            return loopingRangeTicks->getStart();

        return std::nullopt;
    }

    // returns the end of the loop in ticks. if not looping, it returns a nullopt
    [[nodiscard]] std::optional<uint64_t> getLoopingEnd() const
    {
        if (isLooping())
            return loopingRangeTicks->getEnd();

        return std::nullopt;
    }

    [[nodiscard]] uint64_t getCurrentTick() const
    {
        return currentTick;
    }

    // returns the amount of time (in ms) a audio callback represents (the buffer length / sample rate)
    [[nodiscard]] double getDeviceCallbackDurationMs() const
    {
        return blockDurationMs;
    }

    [[nodiscard]] double getTickTimeMs() const
    {
        return tickTimeMs;
    }

    // returns whether the play head has a looping range (which makes it loop automatically)
    [[nodiscard]] bool isLooping() const
    {
        return loopingRangeTicks.has_value();
    }

    // advances the play head by one device callback duration (ms)
    // if one buffer of the device callback represents 10ms of audio,
    // the play head will be moved forward by 10ms
    // this will also take into account the ticks that were scheduled during that device callback
    void advanceDeviceBuffer()
    {
        auto timePoint = 0.0;
        auto tick = currentTick;

        if (timeSinceLastTickMs > 0.0)
            timePoint = tickTimeMs - timeSinceLastTickMs;

        while (timePoint < blockDurationMs - tickTimeMs)
        {
            timePoint += tickTimeMs;
            tick = getTickAfter (tick);
        }

        timeSinceLastTickMs = blockDurationMs - timePoint;
        // tick now represents the last tick in the current buffer, so it should be one after that
        currentTick = getTickAfter (tick);
    }

    uint64_t getTickAfter (uint64_t tick) const
    {
        tick += 1;
        if (isLooping())
            if (tick > loopingRangeTicks->getEnd())
                tick = loopingRangeTicks->getStart();
        return tick;
    }
};


template <typename Function>
auto forEachTick (const PlayHead& playHead, Function&& function)
{
    const auto timeSinceLastTick = playHead.getTimeSinceLastTickMs();
    auto currentTick = playHead.getCurrentTick();
    auto timePointMs = 0.0;

    if (timeSinceLastTick > 0.0)
        timePointMs = playHead.getTickTimeMs() - timeSinceLastTick;

    while (timePointMs < playHead.getDeviceCallbackDurationMs())
    {
        function (currentTick, timePointMs);
        timePointMs += playHead.getTickTimeMs();
        currentTick = playHead.getTickAfter (currentTick);
    }
}