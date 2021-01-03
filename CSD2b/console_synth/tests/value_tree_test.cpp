
// Written by Wouter Ensink

#include <catch2/catch_all.hpp>
#include <console_synth/utility/property.h>
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


TEST_CASE ("property")
{
    auto rootTree = juce::ValueTree ("root");

    auto synthBranch = juce::ValueTree { "synth" };
    rootTree.addChild (synthBranch, -1, nullptr);

    auto frequency = Property<double> { synthBranch, "frequency" };
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


TEST_CASE ("property undo/redo")
{
    auto sequencer = juce::ValueTree { "sequencer" };
    sequencer.setProperty ("tempo", 100, nullptr);
    auto undoManager = juce::UndoManager {};
    auto tempo = Property<int> { sequencer, "tempo" };

    CHECK (tempo.getValue() == 100);

    auto v = 0;
    tempo.onChange = [&v] (auto newValue) {
        v = newValue;
    };

    CHECK (v == 0);

    sequencer.setProperty ("tempo", 200, &undoManager);

    CHECK (v == 200);

    CHECK (undoManager.undo());
    CHECK (v == 100);

    CHECK (undoManager.redo());
    CHECK (v == 200);
}


TEST_CASE ("value tree copy children and properties")
{
    auto engine = juce::ValueTree { "engine" };
    auto sequencer = juce::ValueTree { "sequencer" };
    engine.appendChild (sequencer, nullptr);
    auto tempo = Property<int> { sequencer, "tempo" };

    auto value = 0;
    tempo.onChange = [&value] (auto newValue) { value = newValue; };
    sequencer.setProperty ("tempo", 100, nullptr);

    CHECK (value == 100);

    auto engineCopy = engine.createCopy();

    engineCopy.getChildWithName ("sequencer").setProperty ("tempo", 200, nullptr);

    CHECK (value == 100);

    engine.copyPropertiesAndChildrenFrom (engineCopy, nullptr);
    CHECK ((int) engine.getChildWithName ("sequencer").getProperty ("tempo") == 200);
}

// ========================================================================================
// below a test that incorporates the whole project structure in a simple
// example to verify that it will work as planned


enum class SynthType
{
    sine,
    square,
    saw
};


template <>
struct juce::VariantConverter<SynthType>
{
    static SynthType fromVar (const juce::var& value)
    {
        return static_cast<SynthType> ((int) value);
    }

    static juce::var toVar (SynthType type)
    {
        return static_cast<int> (type);
    }
};


struct Synth
{
    explicit Synth (juce::ValueTree& tree, SynthType type) : type { type },
                                                             state ("synth"),
                                                             amplitude (state, "amplitude", 0.0),
                                                             frequency (state, "frequency", 440)
    {
        tree.addChild (state, -1, nullptr);

        frequency.onChange = [this] (auto freq) {
            phaseDelta = freq / sampleRate;
        };
    }

    SynthType type;
    juce::ValueTree state;
    Property<double> amplitude;
    Property<double> frequency;
    double phase = 0.0;
    double sampleRate = 44100;
    double phaseDelta = 0.0;
};


struct Engine
{
    explicit Engine (juce::ValueTree tree) : synthType { state, "synth_type", SynthType::sine }
    {
        possibleSynths.push_back (std::make_unique<Synth> (state, SynthType::sine));
        possibleSynths.push_back (std::make_unique<Synth> (state, SynthType::square));
        possibleSynths.push_back (std::make_unique<Synth> (state, SynthType::saw));

        tree.addChild (state, -1, nullptr);

        synthType.onChange = [this] (auto type) {
            for (auto&& s : possibleSynths)
                if (s->type == type)
                    currentSynth = s.get();
        };

        currentSynth = possibleSynths[0].get();
    }

    juce::ValueTree state { "engine" };
    Property<SynthType> synthType;
    std::vector<std::unique_ptr<Synth>> possibleSynths {};
    Synth* currentSynth = nullptr;
};


struct ConsoleInterface
{
    template <SynthType type>
    void changeSynthTypeTo()
    {
        state.getChildWithName ("engine")
            .setProperty ("synth_type", juce::VariantConverter<SynthType>::toVar (type), &undoManager);
    }

    void setAmplitude (double amp) const
    {
        for (auto c : state.getChildWithName ("engine"))
            if (c.hasType ("synth"))
                c.setProperty ("amplitude", amp, const_cast<juce::UndoManager*> (&undoManager));
    }

    juce::ValueTree state;
    juce::UndoManager undoManager;
};


TEST_CASE ("general project structure")
{
    auto rootTree = juce::ValueTree { "root" };
    auto engine = Engine { rootTree };
    auto consoleInterface = ConsoleInterface { rootTree };

    CHECK (engine.currentSynth->type == SynthType::sine);

    // now when changing the synth type via the console interface, it should automatically sync with the engine
    consoleInterface.changeSynthTypeTo<SynthType::saw>();

    CHECK (engine.currentSynth->type == SynthType::saw);

    consoleInterface.setAmplitude (10.0);

    CHECK_THAT (((double) engine.currentSynth->amplitude), Catch::Matchers::WithinAbs (10.0, 0.001));

    consoleInterface.undoManager.undo();
    CHECK_THAT (((double) engine.currentSynth->amplitude), Catch::Matchers::WithinAbs (0.0, 0.001));

    consoleInterface.undoManager.undo();
    CHECK (engine.currentSynth->type == SynthType::sine);
}

TEST_CASE ("array property")
{
    auto tree = juce::ValueTree { "parent" };
    auto property = Property<juce::Array<juce::var>> { tree, "array", { 0.0, 1.0, 2.0 } };

    auto propertyChanged = false;
    auto newValue = juce::Array<juce::var> { 1.0, 1.0, 1.0 };

    property.onChange = [&propertyChanged] (auto p) {
        propertyChanged = true;
    };


    CHECK (! propertyChanged);
    tree.setProperty ("array", newValue, nullptr);
    CHECK (propertyChanged);
    CHECK ((double) tree["array"][0] == 1.0);

    newValue = juce::Array<juce::var> { 5.0, 4.0, 3.0 };
    REQUIRE ((double) newValue[0] == 5.0);

    double firstValue = 0;

    property.onChange = [&firstValue] (auto p) {
        firstValue = p[0];
    };

    tree.setProperty ("array", newValue, nullptr);

    CHECK (firstValue == 5.0);
}