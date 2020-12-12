

#include <console_synth/sequencer/playhead.h>

[[nodiscard]] bool PlayHead::isLooping() const noexcept
{
    return loopingRange.has_value();
}



void PlayHead::setLooping (juce::Range<uint64_t> range) noexcept
{
    loopingRange = range;

    if (! range.contains (currentTick))
        rewind();
}

void PlayHead::disableLooping() noexcept
{
    loopingRange = std::nullopt;
}

void PlayHead::advanceTick() noexcept
{
    currentTick += 1;

    if (isLooping())
    {
        if (currentTick > (*loopingRange).getEnd())
            rewind();
    }
}

void PlayHead::rewind() noexcept
{
    if (isLooping())
        currentTick = (*loopingRange).getStart();
    else
        currentTick = 0;
}