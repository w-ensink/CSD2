
// Written by Wouter Ensink

// This is the only solution to switch synths easily through the value tree.

#pragma once

enum struct SynthType
{
    rm,
    fm
};


template <>
struct juce::VariantConverter<SynthType>
{
    static juce::var toVar (SynthType type)
    {
        return static_cast<int> (type);
    }

    static SynthType fromVar (const juce::var& type)
    {
        return static_cast<SynthType> ((int) type);
    }
};

