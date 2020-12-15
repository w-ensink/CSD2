

#include <catch2/catch_all.hpp>
#include <juce_data_structures/juce_data_structures.h>
#include <utility>


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


template <typename T>
class Property : juce::ValueTree::Listener
{
public:
    Property (juce::ValueTree& tree, juce::Identifier id, juce::UndoManager* undoManager)
        : tree { tree }, identifier { std::move (id) }, undoManager { undoManager }
    {
        tree.addListener (this);
    }

    Property (juce::ValueTree& tree, const juce::Identifier& id, juce::UndoManager* undoManager, T initialValue)
        : Property { tree, id, undoManager }, cachedValue { initialValue }
    {
    }

    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        if (treeWhosePropertyHasChanged == tree && property == identifier)
        {
            cachedValue = treeWhosePropertyHasChanged[identifier];
            onChange (cachedValue);
        }
    }

    Property& operator= (T newValue)
    {
        setValue (newValue);
        return *this;
    }

    explicit operator T()
    {
        return cachedValue;
    }

    void setValue (T newValue)
    {
        cachedValue = newValue;
        tree.setPropertyExcludingListener (this, identifier, newValue, undoManager);
    }

    [[nodiscard]] T getValue() const
    {
        return cachedValue;
    }

    std::function<void (T)> onChange = [] (T) {};

private:
    juce::ValueTree tree;
    const juce::Identifier identifier;
    juce::UndoManager* undoManager {};

    T cachedValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Property);
};


TEST_CASE ("property")
{
    auto rootTree = juce::ValueTree ("root");

    auto synthBranch = juce::ValueTree { "synth" };
    rootTree.addChild (synthBranch, -1, nullptr);

    auto frequency = Property<double> { synthBranch, "frequency", nullptr };
    auto callbackValue = 0.0;

    frequency.onChange = [&] (auto freq) {
        callbackValue = freq;
    };

    // when changing the value of the property via another reference to the value tree,
    // a the on change function should be called
    rootTree.getChildWithName ("synth").setProperty ("frequency", 5.0, nullptr);

    CHECK_THAT (frequency.getValue(), Catch::Matchers::WithinAbs (5.0, 0.001));
    CHECK_THAT (callbackValue, Catch::Matchers::WithinAbs (5.0, 0.001));

    // via this way, the on change function shouldn't be called, but the value should be changed
    frequency.setValue (6.0);

    CHECK_THAT (frequency.getValue(), Catch::Matchers::WithinAbs (6.0, 0.001));

    // the on change function should not have been called, so setValue should still be 5.0...
    CHECK_THAT (callbackValue, Catch::Matchers::WithinAbs (5.0, 0.001));

    // assert that operator overload for double works correctly
    auto operatorDoubleCheck = (double) frequency;
    CHECK_THAT (operatorDoubleCheck, Catch::Matchers::WithinAbs (6.0, 0.001));

    // check that assignment works (also shouldn't call onChange lambda)
    frequency = 7.0;
    CHECK_THAT (frequency.getValue(), Catch::Matchers::WithinAbs (7.0, 0.001));
    CHECK_THAT (callbackValue, Catch::Matchers::WithinAbs (5.0, 0.001));
}