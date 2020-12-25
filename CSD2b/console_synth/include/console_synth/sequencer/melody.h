
// Written by Wouter Ensink

#pragma once

#include <console_synth/drow_ValueTreeObjectList.h>
#include <console_synth/identifiers.h>
#include <console_synth/property.h>
#include <console_synth/sequencer/event.h>
#include <utility>

struct Note
{
    explicit Note (juce::ValueTree tree) : state { std::move (tree) }
    {
        midiNoteNumber.referTo (state, IDs::midiNoteNumber, nullptr);
        velocity.referTo (state, IDs::velocity, nullptr);
        timeStampTicks.referTo (state, IDs::startTimeTicks, nullptr);
        lengthInTicks.referTo (state, IDs::lengthTicks, nullptr);
    }

    juce::ValueTree state;
    juce::CachedValue<int> midiNoteNumber;
    juce::CachedValue<int> velocity;
    juce::CachedValue<int> timeStampTicks;
    juce::CachedValue<int> lengthInTicks;

    [[nodiscard]] auto getSequencerEvents() const noexcept -> std::pair<Event, Event>
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


struct Melody : public drow::ValueTreeObjectList<Note>
{
    explicit Melody (juce::ValueTree& tree) : drow::ValueTreeObjectList<Note> { juce::ValueTree { IDs::melody } }
    {
        tree.appendChild (drow::ValueTreeObjectList<Note>::parent, nullptr);
        rebuildObjects();
    }

    ~Melody() override
    {
        freeObjects();
    }

    Note* createNewObject (const juce::ValueTree& tree) override
    {
        auto* n = new Note { tree };
        return n;
    }

    void deleteObject (Note* note) override
    {
        delete note;
    }

    [[nodiscard]] bool isSuitableType (const juce::ValueTree& tree) const override
    {
        return tree.hasType (IDs::note);
    }

    void newObjectAdded (Note*) override
    {
        rebuildEventList();
    }

    void objectRemoved (Note*) override
    {
        rebuildEventList();
    }


    [[nodiscard]] std::vector<Event> getSequencerEvents() const
    {
        return events;
    }

    void objectOrderChanged() override {}

private:
    std::vector<Event> events;

    void rebuildEventList()
    {
        auto newEvents = std::vector<Event> (objects.size() * 2);

        for (auto* note : objects)
        {
            auto [e1, e2] = note->getSequencerEvents();
            newEvents.emplace_back (e1);
            newEvents.emplace_back (e2);
        }

        std::sort (newEvents.begin(), newEvents.end(), [] (auto a, auto b) {
            return a.timeStampTicks > b.timeStampTicks;
        });

        events = std::move (newEvents);
    }
};