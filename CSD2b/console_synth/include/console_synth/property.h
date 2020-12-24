
#pragma once

#include <juce_data_structures/juce_data_structures.h>

template <typename T>
class Property final : private juce::ValueTree::Listener
{
public:
    Property (juce::ValueTree& tree, juce::Identifier id, juce::UndoManager* undoManager, T initialValue = {})
        : tree { tree }, identifier { std::move (id) }, undoManager { undoManager }, cachedValue { initialValue }
    {
        tree.addListener (this);

        if (auto* v = tree.getPropertyPointer (identifier))
            cachedValue = juce::VariantConverter<T>::fromVar (*v);
        else
            tree.setProperty (identifier, juce::VariantConverter<T>::toVar (cachedValue), nullptr);
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
        tree.setPropertyExcludingListener (this, identifier, juce::VariantConverter<T>::toVar (newValue), undoManager);
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


    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        if (treeWhosePropertyHasChanged == tree && property == identifier)
        {
            cachedValue = juce::VariantConverter<T>::fromVar (treeWhosePropertyHasChanged[identifier]);
            onChange (cachedValue);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Property);
};
