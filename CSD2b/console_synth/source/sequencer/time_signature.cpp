
// Written by Wouter Ensink

#include <bitset>
#include <console_synth/sequencer/time_signature.h>

TimeSignature::TimeSignature (uint32_t numerator, uint32_t denominator, uint32_t ticksPerQuarterNote)
{
    setNumerator (numerator);
    setDenominator (denominator);
    setTicksPerQuarterNote (ticksPerQuarterNote);
}


[[nodiscard]] uint32_t TimeSignature::getNumerator() const noexcept
{
    return numerator;
}

[[nodiscard]] uint32_t TimeSignature::getDenominator() const noexcept
{
    return denominator;
}

[[nodiscard]] uint32_t TimeSignature::getTicksPerQuarterNote() const noexcept
{
    return ticksPerQuarterNote;
}

void TimeSignature::setNumerator (uint32_t newNumerator)
{
    if (newNumerator == 0)
        throw InvalidNumeratorError();

    numerator = newNumerator;
}

void TimeSignature::setDenominator (uint32_t newDenominator)
{
    if (! isValidDenominator (newDenominator))
        throw InvalidDenominatorError();

    denominator = newDenominator;
}

void TimeSignature::setTicksPerQuarterNote (uint32_t newTicksPerQuarterNote) noexcept
{
    ticksPerQuarterNote = newTicksPerQuarterNote;
}


bool TimeSignature::isValidDenominator (uint32_t toCheck) noexcept
{
    // a power of 2, but not 1.
    return std::bitset<sizeof (uint32_t) * 8> { toCheck }.count() == 1 && toCheck != 1;
}


[[nodiscard]] uint32_t TimeSignature::getTicksPerBar() const noexcept
{
    return getTicksPerDenominator() * numerator;
}

[[nodiscard]] uint32_t TimeSignature::getTicksPerDenominator() const
{
    auto numDenominatorsInQuarterNote = denominator / 4;
    return ticksPerQuarterNote / numDenominatorsInQuarterNote;
}