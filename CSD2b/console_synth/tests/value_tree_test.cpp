

#include <catch2/catch_all.hpp>
#include <juce_data_structures/juce_data_structures.h>


struct TreeListener : juce::ValueTree::Listener
{
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override
    {
        if (onValueTreeChildAdded)
            onValueTreeChildAdded (parentTree, childWhichHasBeenAdded);
    }

    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        if (onValueTreePropertyHasChanged)
            onValueTreePropertyHasChanged (treeWhosePropertyHasChanged, property);
    }

    std::function<void (juce::ValueTree&, juce::ValueTree&)> onValueTreeChildAdded;
    std::function<void (juce::ValueTree&, const juce::Identifier&)> onValueTreePropertyHasChanged;
};

auto makeNote (int number, int start, int length, int velocity)
{
    auto note = juce::ValueTree { "note" };
    note.setProperty ("number", number, nullptr);
    note.setProperty ("start", start, nullptr);
    note.setProperty ("length", length, nullptr);
    note.setProperty ("velocity", velocity, nullptr);
    return note;
}


TEST_CASE ("value tree test", "[value tree]")
{
    auto rootId = juce::Identifier { "root" };
    auto melody = juce::ValueTree { rootId };

    auto listener = TreeListener();
    melody.addListener (&listener);

    SECTION ("add single note")
    {
        auto additionNoted = false;

        listener.onValueTreeChildAdded = [&additionNoted] (auto parent, auto newChild) {
            additionNoted = true;
        };

        auto note = makeNote (60, 0, 2, 127);
        melody.addChild (note, 0, nullptr);

        REQUIRE (additionNoted);
    }
}
