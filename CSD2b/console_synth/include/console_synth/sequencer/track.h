
#pragma once

#include <console_synth/audio/processor_chain.h>
#include <console_synth/audio/synthesizer.h>
#include <console_synth/midi/midi_source.h>
#include <console_synth/sequencer/event.h>
#include <console_synth/sequencer/render_context.h>


struct Track
{
    Track()
    {
        melody->notes.push_back (Note {
            .midiNoteNumber = 60 + 36,
            .velocity = 100,
            .timeStampTicks = 0,
            .lengthInTicks = 48,
        });

        melody->notes.push_back (Note {
            .midiNoteNumber = 62 + 36,
            .velocity = 100,
            .timeStampTicks = 48,
            .lengthInTicks = 48,
        });
    }

    void prepareToPlay (double sampleRate, int numSamplesPerBlockExpected)
    {
        processorChain.prepareToPlay (sampleRate, numSamplesPerBlockExpected);
        // size is arbitrary, but it's probably enough as the midi buffer will never
        // hold more than one buffer length worth of data.
        midiScratchBuffer.ensureSize (256);
    }

    void renderNextBlock (RenderContext& renderContext)
    {
        midiScratchBuffer.clear();

        if (! renderContext.isStopped())
        {
            melodyPlayerMidiSource.fillNextMidiBuffer (renderContext.getPlayHead(),
                                                       midiScratchBuffer,
                                                       renderContext.getNumSamples());
        }

        // if the track is record enabled and the engine is not exporting to audio:
        // merge external midi with track midi
        if (isRecordEnabled && ! renderContext.isExporting())
        {
            auto& externalMidi = renderContext.getExternalMidi();
            // if the timing is off: experiment with last argument in this call (time offset external midi)
            midiScratchBuffer.addEvents (externalMidi, 0, renderContext.getNumSamples(), 0);
        }

        // keep track of the active midi notes, so we can send noteOff messages when playback stops
        for (auto&& metaMessage : midiScratchBuffer)
        {
            auto midiMessage = metaMessage.getMessage();
            activeMidiNotes.set (midiMessage.getNoteNumber(), midiMessage.isNoteOn());
        }

        // if the sequencer was playing and now it was stopped, some notes might still
        // be active. To prevent them from endlessly playing, we send their corresponding
        // noteOff events
        if (previousPlayState != PlayState::stopped && renderContext.isStopped())
        {
            addMidiOffMessagesForActiveMidiNotes();
        }

        processorChain.processBlock (renderContext.getAudioBuffer(), midiScratchBuffer);
    }

    void addMidiOffMessagesForActiveMidiNotes()
    {
        for (auto i = 0; i < activeMidiNotes.size(); ++i)
            if (activeMidiNotes.test (i))
                midiScratchBuffer.addEvent (juce::MidiMessage::noteOff (1, i), 0);

        activeMidiNotes.reset();
    }

    void releaseResources()
    {
        processorChain.releaseResources();
    }

private:
    Synthesizer synth { 4 };
    ProcessorChain processorChain { synth };
    std::bitset<128> activeMidiNotes { 0 };
    std::unique_ptr<Melody> melody = std::make_unique<Melody>();
    MelodyPlayerMidiSource melodyPlayerMidiSource { melody.get() };
    juce::String name;
    bool isRecordEnabled = true;
    juce::MidiBuffer midiScratchBuffer;
    PlayState previousPlayState = PlayState::stopped;
};
