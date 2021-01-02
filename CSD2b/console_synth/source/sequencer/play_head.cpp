
// Written by Wouter Ensink

#include <console_synth/sequencer/play_head.h>


void PlayHead::setTickTimeMs (double tickTime)
{
    tickTimeMs = tickTime;
    timeSinceLastTickMs = tickTime;
}

void PlayHead::setDeviceCallbackDurationMs (double duration)
{
    blockDurationMs = duration;
}

void PlayHead::setPositionInTicks (uint64_t position)
{
    currentTick = position;
}

void PlayHead::setTimeSinceLastTickMs (double time)
{
    timeSinceLastTickMs = time;
}


[[nodiscard]] double PlayHead::getTimeSinceLastTickMs() const
{
    return timeSinceLastTickMs;
}


void PlayHead::setLooping (uint64_t start, uint64_t end)
{
    loopingRangeTicks = { start, end };

    if (! (*loopingRangeTicks).contains (currentTick))
    {
        currentTick = start;
    }
}


[[nodiscard]] std::optional<uint64_t> PlayHead::getLoopingStart() const
{
    if (isLooping())
        return loopingRangeTicks->getStart();

    return std::nullopt;
}


[[nodiscard]] std::optional<uint64_t> PlayHead::getLoopingEnd() const
{
    if (isLooping())
        return loopingRangeTicks->getEnd();

    return std::nullopt;
}

[[nodiscard]] uint64_t PlayHead::getCurrentTick() const
{
    return currentTick;
}


[[nodiscard]] double PlayHead::getDeviceCallbackDurationMs() const
{
    return blockDurationMs;
}

[[nodiscard]] double PlayHead::getTickTimeMs() const
{
    return tickTimeMs;
}


[[nodiscard]] bool PlayHead::isLooping() const
{
    return loopingRangeTicks.has_value();
}


void PlayHead::advanceDeviceBuffer()
{
    auto timePoint = tickTimeMs - timeSinceLastTickMs;
    auto tick = currentTick;

    while (timePoint < blockDurationMs)
    {
        timePoint += tickTimeMs;
        tick = getTickAfter (tick);
    }

    timeSinceLastTickMs = blockDurationMs - (timePoint - tickTimeMs);
    // tick now represents the last tick in the current buffer, so it should be one after that
    currentTick = tick;
}


[[nodiscard]] uint64_t PlayHead::getTickAfter (uint64_t tick) const
{
    tick += 1;
    if (isLooping())
        if (tick >= loopingRangeTicks->getEnd())
            tick = loopingRangeTicks->getStart();
    return tick;
}