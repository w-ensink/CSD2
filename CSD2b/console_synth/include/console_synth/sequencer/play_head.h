
// Written by Wouter Ensink

#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>

/* This is the play head of the sequencer, this was a really tricky one to get right. The design is as follows:
 * Time in this sequencer is based on the audio device callback. It is the best option, because it is already synced
 * with the audio thread (so no annoying threading issues with timing), it is deterministic (with a system clock
 * you can't know for sure at what point (what processor/how far into the current buffer) the audio callback is,
 * it's very regular and it allows for non realtime rendering.
 *
 * This means the play head needs to know about the duration of one audio device callback buffer and it has to
 * know the duration of a single tick. During a device callback, the play head can move only once. This is because
 * it will have to be passed into multiple processors, that all operate on the same buffer. To allow each processor to
 * know at what point the play head is into the session at any point in the current buffer, there is a utility function
 * called forEachTick(). This takes the play head and a function (lambda or any other callable object) and then calls that function
 * with each new tick and the corresponding time point relative to the current buffer. This also takes looping into account.
 * */

class PlayHead
{
public:
    PlayHead() = default;

    void setTickTimeMs (double tickTime)
    {
        tickTimeMs = tickTime;
        timeSinceLastTickMs = tickTime;
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

    // looping from start to (excluding) end
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

    [[nodiscard]] uint64_t getTickAfter (uint64_t tick) const
    {
        tick += 1;
        if (isLooping())
            if (tick >= loopingRangeTicks->getEnd())
                tick = loopingRangeTicks->getStart();
        return tick;
    }

private:
    double tickTimeMs = 0;
    double blockDurationMs = 0;
    uint64_t currentTick = 0;
    double timeSinceLastTickMs = 0;
    std::optional<juce::Range<uint64_t>> loopingRangeTicks = std::nullopt;
};

// ===================================================================================================

template <typename Functor>
auto forEachTick (const PlayHead& playHead, Functor&& function)
{
    auto currentTick = playHead.getCurrentTick();
    auto timePointMs = playHead.getTickTimeMs() - playHead.getTimeSinceLastTickMs();

    while (timePointMs < playHead.getDeviceCallbackDurationMs())
    {
        function (currentTick, timePointMs);
        timePointMs += playHead.getTickTimeMs();
        currentTick = playHead.getTickAfter (currentTick);
    }
}