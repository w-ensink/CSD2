

// structure that represents an event that can be scheduled by the sequencer
//

#pragma once

#include <console_synth/sequencer/time_signature.h>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

struct Event
{
    uint8_t midiNote;
    uint8_t velocity;
    uint64_t timeStampTicks;
    bool isNoteOn;
};


struct Note
{
    uint8_t midiNoteNumber;
    uint8_t velocity;
    uint64_t timeStampTicks;
    uint64_t lengthInTicks;

    auto getSequencerEvents() const noexcept -> std::pair<Event, Event>
    {
        return {
            Event {
                .midiNote = midiNoteNumber,
                .velocity = velocity,
                .timeStampTicks = timeStampTicks,
                .isNoteOn = true },
            Event {
                .midiNote = midiNoteNumber,
                .velocity = velocity,
                .timeStampTicks = timeStampTicks + lengthInTicks,
                .isNoteOn = false }
        };
    }
};


struct Melody
{
    std::vector<Note> notes;
    TimeSignature timeSignature;

    std::vector<Event> getSequencerEvents() const
    {
        auto result = std::vector<Event> (notes.size() * 2);

        std::for_each (notes.begin(), notes.end(), [&result] (const auto& note) {
            auto [e1, e2] = note.getSequencerEvents();
            result.push_back (e1);
            result.push_back (e2);
        });

        std::sort (result.begin(), result.end(), [] (auto a, auto b) {
            return a.timeStampTicks > b.timeStampTicks;
        });

        return result;
    }
};