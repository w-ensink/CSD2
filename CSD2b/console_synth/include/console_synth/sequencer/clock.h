
#pragma once

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

class Clock
{
public:
    Clock() = delete;

    Clock (std::chrono::milliseconds tickTimeMs) : tickTimeMs (tickTimeMs) {}

    void start() noexcept
    {
        startTime = ClockType::now();
        targetTime = startTime + tickTimeMs;
    }

    void setTickTimeMs (std::chrono::milliseconds newTickTimeMs) noexcept
    {
        tickTimeMs = newTickTimeMs;
    }

    void blockUntilNextTick()
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