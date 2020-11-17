
// Written by Wouter Ensink

#include <juce_core/juce_core.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <fmt/format.h>


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