
// Written by Wouter Ensink

#pragma once

#include <fmt/format.h>
#include <juce_core/juce_core.h>

// partial template specialization to make fmt work with juce::String
template <>
struct fmt::formatter<juce::String>
{
    template <typename ParseContext>
    constexpr auto parse (ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format (const juce::String& toFormat, FormatContext& context)
    {
        return fmt::format_to (context.out(), "{}", toFormat.toRawUTF8());
    }
};