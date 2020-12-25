
// Written by Wouter Ensink

#pragma once

#include <cstdint>
#include <exception>


class TimeSignature
{
public:
    TimeSignature() = delete;

    TimeSignature (uint32_t numerator, uint32_t denominator, uint32_t ticksPerQuarterNote);


    struct InvalidNumeratorError;
    struct InvalidDenominatorError;


    [[nodiscard]] uint32_t getNumerator() const noexcept;

    [[nodiscard]] uint32_t getDenominator() const noexcept;

    [[nodiscard]] uint32_t getTicksPerQuarterNote() const noexcept;

    void setNumerator (uint32_t newNumerator);

    void setDenominator (uint32_t newDenominator);

    void setTicksPerQuarterNote (uint32_t newTicksPerQuarterNote) noexcept;

    static bool isValidDenominator (uint32_t toCheck) noexcept;

    [[nodiscard]] uint32_t getTicksPerBar() const noexcept
    {
        return getTicksPerDenominator() * numerator;
    }

    [[nodiscard]] uint32_t getTicksPerDenominator() const
    {
        auto numDenominatorsInQuarterNote = denominator / 4;
        return ticksPerQuarterNote / numDenominatorsInQuarterNote;
    }

private:
    uint32_t numerator { 4 };
    uint32_t denominator { 4 };
    uint32_t ticksPerQuarterNote { 4 };
};


struct TimeSignature::InvalidNumeratorError : std::exception
{
    [[nodiscard]] const char* what() const noexcept override
    {
        return "invalid numerator in time signature";
    }
};


struct TimeSignature::InvalidDenominatorError : std::exception
{
    [[nodiscard]] const char* what() const noexcept override
    {
        return "invalid denominator in time signature";
    }
};
