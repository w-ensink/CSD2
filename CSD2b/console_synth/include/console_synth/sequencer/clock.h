
#pragma once

#include <chrono>
#include <thread>


class SequencerClockBase
{
public:
    virtual ~SequencerClockBase() = default;
    virtual void start() = 0;
    virtual void setTickTimeMs (std::chrono::milliseconds) = 0;
    virtual void blockUntilNextTick() = 0;
};

// =====================================================

using namespace std::chrono_literals;

class Clock : public SequencerClockBase
{
public:
    Clock() = delete;

    explicit Clock (std::chrono::milliseconds tickTimeMs) : tickTimeMs (tickTimeMs) {}

    void start() noexcept override
    {
        startTime = ClockType::now();
        targetTime = startTime + tickTimeMs;
    }

    void setTickTimeMs (std::chrono::milliseconds newTickTimeMs) noexcept override
    {
        tickTimeMs = newTickTimeMs;
    }

    void blockUntilNextTick() override
    {
        while (ClockType::now() < targetTime)
        {
            std::this_thread::sleep_for (100ns);
        }

        targetTime += tickTimeMs;
    }

private:
    using ClockType = std::chrono::high_resolution_clock;

    std::chrono::milliseconds tickTimeMs;
    ClockType::time_point startTime;
    ClockType::time_point targetTime;
};