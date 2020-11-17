
// Written by Wouter Ensink

#include <fmt/format.h>
#include <juce_core/juce_core.h>


int main()
{
    auto w = juce::String ("World");
    fmt::print ("Hello {}\n", w.toRawUTF8());

    return 0;
}