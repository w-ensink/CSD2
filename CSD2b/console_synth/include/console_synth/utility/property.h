
// Written by Wouter Ensink

#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include <utility>

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


// partial template specialization for juce::Array<juce::var>, because that works a little different
template <>
class Property<juce::Array<juce::var>> : public juce::ValueTree::Listener
{
public:
    Property (juce::ValueTree& tree, juce::Identifier id, juce::Array<juce::var> initialValue = {})
        : tree { tree }, identifier { std::move (id) }, cachedValue { std::move (initialValue) }
    {
        tree.addListener (this);

        if (auto* v = tree.getPropertyPointer (identifier))
            cachedValue = *v;
        else
            tree.setProperty (identifier, cachedValue, nullptr);
    }

    Property& operator= (const juce::Array<juce::var>& newValue)
    {
        setValue (newValue);
        return *this;
    }

    explicit operator juce::Array<juce::var>()
    {
        return cachedValue;
    }

    void setValue (const juce::Array<juce::var>& newValue)
    {
        cachedValue = newValue;
        tree.setPropertyExcludingListener (this, identifier, newValue, nullptr);
    }

    [[nodiscard]] juce::var getValue() const
    {
        return cachedValue;
    }

    juce::var operator[] (int index)
    {
        return cachedValue[index];
    }

    std::function<void (const juce::var&)> onChange = [] (const juce::var&) {};

private:
    juce::ValueTree tree;
    const juce::Identifier identifier;
    juce::var cachedValue;


    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        if (treeWhosePropertyHasChanged == tree && property == identifier)
        {
            cachedValue = treeWhosePropertyHasChanged[identifier];
            onChange (cachedValue);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Property);
};

using ArrayProperty = Property<juce::Array<juce::var>>;