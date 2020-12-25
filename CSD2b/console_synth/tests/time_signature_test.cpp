
// Written by Wouter Ensink

#include <catch2/catch_all.hpp>
#include <console_synth/sequencer/time_signature.h>

TEST_CASE ("time signature", "basic setup")
{
    auto ts = TimeSignature { 4, 4, 4 };

    REQUIRE (ts.getNumerator() == 4);
    REQUIRE (ts.getDenominator() == 4);
    REQUIRE (ts.getTicksPerQuarterNote() == 4);

    SECTION ("init with different spec")
    {
        auto ts2 = TimeSignature { 5, 8, 16 };

        REQUIRE (ts2.getNumerator() == 5);
        REQUIRE (ts2.getDenominator() == 8);
        REQUIRE (ts2.getTicksPerQuarterNote() == 16);
    }

    SECTION ("change numerator (valid)")
    {
        ts.setNumerator (8);
        REQUIRE (ts.getNumerator() == 8);
    }

    SECTION ("change numerator (invalid)")
    {
        REQUIRE_THROWS_AS (ts.setNumerator (0), TimeSignature::InvalidNumeratorError);
    }

    SECTION ("change denominator (valid)")
    {
        ts.setDenominator (8);
        REQUIRE (ts.getDenominator() == 8);
    }

    SECTION ("change denominator (invalid)")
    {
        REQUIRE_THROWS_AS (ts.setDenominator (0), TimeSignature::InvalidDenominatorError);
        REQUIRE_THROWS_AS (ts.setDenominator (3), TimeSignature::InvalidDenominatorError);
        REQUIRE_THROWS_AS (ts.setDenominator (1), TimeSignature::InvalidDenominatorError);
    }
}