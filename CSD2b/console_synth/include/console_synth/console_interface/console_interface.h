
#pragma once

#include <ctre.hpp>
#include <optional>
#include <console_synth/format.h>

std::optional<std::string_view> attemptSetMidiCommand (std::string_view s) noexcept
{
    static constexpr auto pattern = ctll::fixed_string { "^set\\smidi\\s([a-zA-Z0-9]+)$" };

    if (auto m = ctre::match<pattern> (s))
        return m.get<1>().to_view();

    return std::nullopt;
}

auto isQuitCommand (std::string_view s) noexcept
{
    static constexpr auto pattern = ctll::fixed_string { "^(quit|exit)$" };

    if (ctre::match<pattern> (s))
        return true;

    return false;
}

auto fetchUserInput (const std::string& message)
{
    fmt::print ("{}", message);
    auto targetString = std::string {};
    std::getline (std::cin, targetString);
    return targetString;
}