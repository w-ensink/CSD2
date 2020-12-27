
// Written by Wouter Ensink

#include <juce_audio_processors/juce_audio_processors.h>

// just a wrapper around the juce adsr class that calls a lambda when the release stage is finished.
// this is useful if you want to turn off a synth voice after the envelope is done
class ADSR
{
public:
    float getNextSample()
    {
        if ((! internalAdsr.isActive()) && wasActive)
            onReleaseDone();

        wasActive = internalAdsr.isActive();
        return internalAdsr.getNextSample();
    }

    void noteOn()
    {
        internalAdsr.noteOn();
    }

    void noteOff()
    {
        internalAdsr.noteOff();
    }

    void setParameters (juce::ADSR::Parameters params)
    {
        internalAdsr.setParameters (params);
    }

    void setSampleRate (double rate)
    {
        internalAdsr.setSampleRate (rate);
    }

    void setReleaseFinishedCallback (const std::function<void()>& fn)
    {
        onReleaseDone = fn;
    }

    [[nodiscard]] bool isActive() const noexcept { return internalAdsr.isActive(); }

    void reset()
    {
        internalAdsr.reset();
        wasActive = false;
    }

private:
    std::function<void()> onReleaseDone = [] {};
    juce::ADSR internalAdsr;
    bool wasActive = false;
};