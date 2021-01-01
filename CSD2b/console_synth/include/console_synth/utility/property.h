
// Written by Wouter Ensink

#pragma once

#include <juce_data_structures/juce_data_structures.h>

/* This is a wrapper around a value tree property, to make it more usable. This class
 * listens to changes in the value of the property inside the value tree. When
 * a change in value occurs, it will call the callback that was assigned to it with the new value.
 * */

template <typename T>
class Property final : private juce::ValueTree::Listener
{
public:
    Property (juce::ValueTree& tree, juce::Identifier id, T initialValue = {})
        : tree { tree }, identifier { std::move (id) }, cachedValue { initialValue }
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
        tree.setPropertyExcludingListener (this, identifier, juce::VariantConverter<T>::toVar (newValue), nullptr);
    }

    [[nodiscard]] T getValue() const
    {
        return cachedValue;
    }

    std::function<void (T)> onChange = [] (T) {};

private:
    juce::ValueTree tree;
    const juce::Identifier identifier;
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
