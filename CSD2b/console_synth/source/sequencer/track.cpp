
// Written by Wouter Ensink

#include <console_synth/sequencer/track.h>


Track::Track (juce::ValueTree parent)
{
    parent.appendChild (trackState, nullptr);

    synthType.onChange = [this] (auto newType) {
        if (newType == SynthType::fm && dynamic_cast<FmSynthesizer*> (synth.get()) == nullptr)
            switchSynth<FmSynthesizer>();

        else if (newType == SynthType::rm && dynamic_cast<RmSynthesizer*> (synth.get()) == nullptr)
            switchSynth<RmSynthesizer>();
    };
}


void Track::prepareToPlay (double newSampleRate, int numSamplesPerBlockExpected)
{
    sampleRate = newSampleRate;

    {
        auto synthLock = std::scoped_lock { synthMutex };
        synth->prepareToPlay (sampleRate, numSamplesPerBlockExpected);
    }
    // size is arbitrary, but it's probably enough as the midi buffer will never
    // hold more than one buffer length worth of data.
    midiScratchBuffer.ensureSize (256);
}


void Track::renderNextBlock (RenderContext& renderContext)
{
    midiScratchBuffer.clear();

    // if playback is not stopped, the midi buffer should be filled with the melody on this track
    if (! renderContext.isStopped())
        addInternalMidiToScratchBuffer (renderContext);

    // if the track is record enabled and the engine is not exporting to audio:
    // merge external midi with track midi
    if (isRecordEnabled && ! renderContext.isExporting())
        addExternalMidiToScratchBuffer (renderContext);


    // keep track of the active midi notes, so we can send noteOff messages when playback stops
    updateActiveMidiNotes();

    // if the sequencer was playing and now it was stopped, some notes might still
    // be active. To prevent them from endlessly playing, we send their corresponding
    // noteOff events
    if (previousPlayState != PlayState::stopped && renderContext.isStopped())
        addMidiOffMessagesForActiveMidiNotes (0);

    // when the play head reaches the end of a loop, it should fire note off events for all active midi notes
    // to prevent them from going on forever
    if (renderContext.getPlayHead().isLooping())
    {
        forEachTick (renderContext.getPlayHead(), [this, &renderContext] (uint64_t tick, double timeRelativeToBuffer) {
            if (tick == *renderContext.getPlayHead().getLoopingEnd() - 1)
            {
                auto sample = (int) (timeRelativeToBuffer / renderContext.getPlayHead().getDeviceCallbackDurationMs()) * renderContext.getNumSamples();
                addMidiOffMessagesForActiveMidiNotes (sample);
            }
        });
    }

    // render the next audio block with the synth, given all relevant midi data for this callback
    {
        auto synthLock = std::scoped_lock { synthMutex };
        synth->processBlock (renderContext.getAudioBuffer(), midiScratchBuffer);
    }

    previousPlayState = renderContext.getPlayState();
}


void Track::releaseResources()
{
    auto synthLock = std::scoped_lock { synthMutex };
    synth->releaseResources();
}


void Track::addInternalMidiToScratchBuffer (const RenderContext& renderContext)
{
    melodyPlayerMidiSource.fillNextMidiBuffer (renderContext.getPlayHead(),
                                               midiScratchBuffer,
                                               renderContext.getNumSamples());
}


void Track::addExternalMidiToScratchBuffer (const RenderContext& renderContext)
{
    // if the timing is off: experiment with last argument in this call (time offset external midi)
    midiScratchBuffer.addEvents (renderContext.getExternalMidi(), 0, renderContext.getNumSamples(), 0);
}


void Track::updateActiveMidiNotes()
{
    for (auto&& metaMessage : midiScratchBuffer)
    {
        auto midiMessage = metaMessage.getMessage();
        activeMidiNotes.set (midiMessage.getNoteNumber(), midiMessage.isNoteOn());
    }
}


void Track::addMidiOffMessagesForActiveMidiNotes (int sample)
{
    for (auto i = 0; i < activeMidiNotes.size(); ++i)
        if (activeMidiNotes.test (i))
            midiScratchBuffer.addEvent (juce::MidiMessage::noteOff (1, i), sample);

    activeMidiNotes.reset();
}
