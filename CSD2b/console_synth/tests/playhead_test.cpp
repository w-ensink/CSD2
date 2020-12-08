

#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/playhead.h>


TEST_CASE ("basic init", "[play head]")
{
    auto ph = PlayHead {};

    REQUIRE (ph.isLooping() == false);
    REQUIRE (ph.getPositionInTicks() == 0);
}

TEST_CASE ("enable looping", "[play head]")
{
    auto ph = PlayHead();
    REQUIRE (ph.isLooping() == false);

    ph.setLooping ({ 0, 10 });
    REQUIRE (ph.isLooping() == true);
}

TEST_CASE ("disable looping", "[play head]")
{
    auto ph = PlayHead();
    ph.setLooping ({ 0, 10 });
    REQUIRE (ph.isLooping() == true);

    ph.disableLooping();
    REQUIRE (ph.isLooping() == false);
}

TEST_CASE ("simple advancing", "[play head]")
{
    auto ph = PlayHead();
    REQUIRE (ph.getPositionInTicks() == 0);

    ph.advanceTick();
    REQUIRE (ph.getPositionInTicks() == 1);

    for (auto i = 0; i < 9; ++i)
        ph.advanceTick();

    REQUIRE (ph.getPositionInTicks() == 10);
}

TEST_CASE ("simple non-looping rewind", "[play head]")
{
    auto ph = PlayHead();
    REQUIRE (ph.getPositionInTicks() == 0);

    for (auto i = 0; i < 10; ++i)
        ph.advanceTick();

    REQUIRE (ph.getPositionInTicks() == 10);

    ph.rewind();

    REQUIRE (ph.getPositionInTicks() == 0);
}

TEST_CASE ("advance past looping point", "[play head]")
{
    auto ph = PlayHead();
    ph.setLooping ({ 0, 10 });

    for (auto i = 0; i < 10; ++i)
        ph.advanceTick();

    REQUIRE (ph.getPositionInTicks() == 10);

    ph.advanceTick();

    REQUIRE (ph.getPositionInTicks() == 0);
}

TEST_CASE ("looping range starting non-zero", "[play head]")
{
    auto ph = PlayHead();
    ph.setLooping ({ 10, 20 });
    REQUIRE (ph.getPositionInTicks() == 10);

    // 11 ticks later, it should be on 10 again...
    for (auto i = 0; i < 11; ++i)
        ph.advanceTick();

    REQUIRE (ph.getPositionInTicks() == 10);

    ph.advanceTick();
    ph.rewind();
    REQUIRE (ph.getPositionInTicks() == 10);

    ph.disableLooping();
    REQUIRE (ph.getPositionInTicks() == 10);
}