
// Written by Wouter Ensink

#pragma once

#include <console_synth/identifiers.h>
#include <console_synth/sequencer/event.h>
#include <console_synth/utility/drow_ValueTreeObjectList.h>
#include <console_synth/utility/property.h>
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


struct Melody : private drow::ValueTreeObjectList<Note>
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

    // Thread safe way of applying a function to each event in the melody.
    // The reason for this construct is that returning a reference to the events
    // could result in a dangling reference if a new melody was generated while the
    // sequencer was still using the old sequencer events.
    // and returning a copy of the events is very expensive, since it requires a heap allocation.
    // the function has to acquire the lock, so it might have to wait a very small time.
    template <typename Functor>
    void forEachEvent (Functor&& function)
    {
        auto lock = std::scoped_lock { eventsMutex };
        std::for_each (events.begin(), events.end(), std::forward<Functor> (function));
    }

private:
    std::vector<Event> events;
    std::mutex eventsMutex;

    void rebuildEventList()
    {
        auto newEvents = std::vector<Event> (objects.size() * 2);

        for (auto* note : objects)
        {
            auto [e1, e2] = note->getSequencerEvents();
            newEvents.emplace_back (e1);
            newEvents.emplace_back (e2);
        }

        // when a note on and note off event for the same note at the same tick occur,
        // it should first handle the note off and then the note on
        std::sort (newEvents.begin(), newEvents.end(), [] (const auto& a, const auto& b) {
            if (a.timeStampTicks == b.timeStampTicks && a.midiNote == b.midiNote)
                return ! a.isNoteOn;

            return a.timeStampTicks > b.timeStampTicks;
        });

        auto eventsLock = std::scoped_lock { eventsMutex };
        events = std::move (newEvents);
    }


    Note* createNewObject (const juce::ValueTree& tree) override
    {
        return new Note { tree };
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

    void objectOrderChanged() override {}
};