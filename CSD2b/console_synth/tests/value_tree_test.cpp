

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

        listener.onValueTreeChildAdded = [&additionNoted] (auto, auto) { additionNoted = true; };

        auto note = makeNote (60, 0, 2, 127);
        melody.addChild (note, 0, nullptr);

        REQUIRE (additionNoted);
    }

    SECTION ("add multiple notes")
    {
        auto addCount = 0;
        listener.onValueTreeChildAdded = [&addCount] (auto, auto) { addCount += 1; };

        melody.addChild (makeNote (60, 0, 2, 127), -1, nullptr);
        melody.addChild (makeNote (62, 2, 2, 127), -1, nullptr);

        REQUIRE (addCount == 2);
    }

    SECTION ("edit note")
    {
        auto note = makeNote (60, 0, 2, 127);
        melody.addChild (note, -1, nullptr);

        auto changeNoted = false;
        listener.onValueTreePropertyHasChanged = [&changeNoted] (auto, auto) {
            changeNoted = true;
        };

        note.setProperty ("number", 50, nullptr);

        REQUIRE (changeNoted);
    }

    SECTION ("child list retrieval")
    {
        auto noteOne = makeNote (60, 0, 2, 127);
        auto noteTwo = makeNote (62, 2, 2, 127);

        melody.addChild (noteOne, -1, nullptr);
        melody.addChild (noteTwo, -1, nullptr);

        for (auto note : melody)
        {
            if (note.getType() == juce::Identifier { "note" })
            {
                auto num = (int) note.getProperty ("number");
                REQUIRE ((num == 60 || num == 62));
            }
        }
    }
}
