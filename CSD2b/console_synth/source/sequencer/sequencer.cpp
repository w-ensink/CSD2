
// Written by Wouter Ensink

#include <console_synth/sequencer/sequencer.h>


Sequencer::Sequencer (juce::ValueTree& parent)
{
    playHead.setLooping (0, timeSignature.getTicksPerBar() + 1);
    setTempoBpm (tempoBpm.getValue());
    parent.appendChild (sequencerState, nullptr);

    tempoBpm.onChange = [this] (auto newTempo) { setTempoBpm (newTempo); };
    midiMessageCollector.ensureStorageAllocated (256);
}


void Sequencer::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    setSampleRate (newSampleRate);
    track.prepareToPlay (sampleRate, samplesPerBlockExpected);
    midiMessageCollector.reset (sampleRate);
}


void Sequencer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // ensure the play head has the right callback duration set
    const auto callbackDurationMs = (double) (bufferToFill.numSamples / sampleRate) * 1000;

    if (callbackDurationMs != playHead.getDeviceCallbackDurationMs())
        playHead.setDeviceCallbackDurationMs (callbackDurationMs);

    // clear the destination buffer for the external midi messages
    midiBuffer.clear();

    // fetch the incoming midi messages from external midi
    midiMessageCollector.removeNextBlockOfMessages (midiBuffer, bufferToFill.numSamples);

    // prepare the render context for the current render pass
    auto renderContext = RenderContext {
        *bufferToFill.buffer,
        midiBuffer,
        playHead,
        playState,
        sampleRate,
        { 0, callbackDurationMs },
        timeSignature
    };

    // render over all tracks...
    track.renderNextBlock (renderContext);

    // move play head one block ahead to prepare for the next callback
    if (playState != PlayState::stopped)
        playHead.advanceDeviceBuffer();
}


void Sequencer::releaseResources()
{
    track.releaseResources();
}


bool Sequencer::openMidiInputDevice (const juce::String& name)
{
    for (auto& d : juce::MidiInput::getAvailableDevices())
    {
        if (d.name.equalsIgnoreCase (name))
        {
            if (auto newInput = juce::MidiInput::openDevice (d.identifier, &midiMessageCollector))
            {
                if (currentMidiInput != nullptr)
                    currentMidiInput->stop();

                currentMidiInput = std::move (newInput);
                currentMidiInput->start();
                return true;
            }
        }
    }

    return false;
}

void Sequencer::stopPlayback()
{
    playState = PlayState::stopped;
}

void Sequencer::startPlayback()
{
    playState = PlayState::playing;
}

[[nodiscard]] bool Sequencer::isPlaying() const noexcept
{
    return playState == PlayState::playing;
}

const TimeSignature& Sequencer::getTimeSignature() const noexcept
{
    return timeSignature;
}

void Sequencer::setTempoBpm (double bpm)
{
    auto ticksPerMinute = bpm * timeSignature.getTicksPerQuarterNote();
    auto msPerTick = 60'000 / ticksPerMinute;
    playHead.setTickTimeMs (msPerTick);
}

void Sequencer::setSampleRate (double rate)
{
    sampleRate = rate;
}