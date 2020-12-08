
#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/sequencer.h>

// this clock implementation doesn't block to wait for a new tick
// to make the tests run really fast
class TestClock : public SequencerClockBase
{
public:
    void start() override {}
    void setTickTimeMs (std::chrono::milliseconds tt) override { tickTime = tt; }
    void blockUntilNextTick() override {}

    std::chrono::milliseconds tickTime = 0ms;
};


TEST_CASE ("basic setup", "[sequencer]")
{
    auto sequencer = Sequencer();
    auto clock = std::make_unique<TestClock>();
    sequencer.setCustomClock (std::move (clock));
}