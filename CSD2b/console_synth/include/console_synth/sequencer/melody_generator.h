
// Written by Wouter Ensink

#pragma once

#include <console_synth/sequencer/melody.h>
#include <console_synth/sequencer/time_signature.h>
#include <juce_data_structures/juce_data_structures.h>

class MelodyGenerator
{
public:
    struct Note
    {
        int number, start, length, velocity;
    };

    juce::ValueTree generateMelody (const TimeSignature& timeSignature)
    {
        auto ticks = timeSignature.getTicksPerBar();
        auto result = juce::ValueTree { IDs::melody };

        auto ticksPerNote = timeSignature.getTicksPerQuarterNote() / 2;
        auto numNotes = timeSignature.getNumerator() * 2;

        auto relativeNotes = generateRelativeNotes (numNotes);
        auto absoluteNoteNumbers = std::vector<int> {};
        absoluteNoteNumbers.reserve (relativeNotes.size());

        auto offset = random.nextInt ({ 60, 80 });

        for (auto n : relativeNotes)
            absoluteNoteNumbers.push_back (n + offset);

        auto tick = 0;

        for (auto num : absoluteNoteNumbers)
        {
            auto note = Note { num, tick, (int) ticksPerNote, 127 };
            addNoteToTree (result, note);
            tick += ticksPerNote;
        }

        return result;
    }

    static void addNoteToTree (juce::ValueTree& tree, const Note& note)
    {
        auto child = juce::ValueTree { IDs::note };
        child.setProperty (IDs::midiNoteNumber, note.number, nullptr);
        child.setProperty (IDs::startTimeTicks, note.start, nullptr);
        child.setProperty (IDs::lengthTicks, note.length, nullptr);
        child.setProperty (IDs::velocity, note.velocity, nullptr);
        tree.appendChild (child, nullptr);
    }


    std::vector<int> generateRelativeNotes (int numNotes) noexcept
    {
        auto notes = std::vector<int> {};
        auto distances = std::vector { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4 };
        auto groundIndex = random.nextInt ({ 1, 5 });
        auto currentNoteIndex = groundIndex;
        notes.push_back (normalizedMidiNoteDistances[currentNoteIndex]);

        for (int i = 1; i < numNotes; ++i)
        {
            auto direction = random.nextBool() ? 1 : -1;
            auto interval = direction * distances[random.nextInt (distances.size())];
            currentNoteIndex = juce::jlimit (0, (int) normalizedMidiNoteDistances.size() - 1, currentNoteIndex + interval);
            notes.push_back (normalizedMidiNoteDistances[currentNoteIndex]);
        }

        std::reverse (std::begin (notes), std::end (notes));

        return notes;
    }


    juce::Random random;
    const std::vector<int> normalizedMidiNoteDistances = { 0, 2, 4, 5, 7, 9, 11 };
};