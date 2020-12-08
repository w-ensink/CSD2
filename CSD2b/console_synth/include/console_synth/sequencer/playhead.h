
#pragma once

#include <cstdint>
#include <juce_core/juce_core.h>

class PlayHead
{
public:
    PlayHead() = default;
    ~PlayHead() = default;

    [[nodiscard]] bool isLooping() const noexcept;

    [[nodiscard]] uint64_t getPositionInTicks() const noexcept;

    void setLooping (juce::Range<uint64_t> range) noexcept;

    void disableLooping() noexcept;

    void advanceTick() noexcept;

    void rewind() noexcept;

private:
    template <typename T>
    using OptionalRange = std::optional<juce::Range<T>>;

    uint64_t currentTick = 0;
    OptionalRange<uint64_t> loopingRange = std::nullopt;
};