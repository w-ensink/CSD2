# Written by Wouter Ensink

# This file contains classes that can be used by the Engine to export sequences to formats like json or midi files.
# This is a big part of the extensability of the engine. Creating a new class should be fairly straight forward.


import mido
import json
from core.sample import Sample, SpectralPositions
from core.session import Session
from core.utility import convert_session_to_dictionary
from core.importers import JsonFileSessionImporter
from copy import copy
from midiutil.MidiFile import MIDIFile


# session exporter that exports to midi using the midiutil module
class MidiUtil_MidiFileSessionExporter:
    def store_session(self, session: Session, file_path: str):
        midi_file = MIDIFile(1)
        track, channel = 0, 9

        self.add_session_meta_data_to_midi_file(midi_file, session)
        notes = self.distribute_midi_notes(session.samples)

        for e in session.events:
            midi_file.addNote(track,
                              channel,
                              notes[e.sample.name],
                              e.time_stamp / session.time_signature.ticks_per_quarter_note,
                              e.duration,
                              e.velocity)

        with open(file_path, 'wb') as f:
            midi_file.writeFile(f)

    @staticmethod
    def add_session_meta_data_to_midi_file(midi_file: MIDIFile, session: Session):
        track, time = 0, 0
        midi_file.addTrackName(track, time, 'Wouter\'s Beat Generator (tm)')
        midi_file.addTimeSignature(track, time,
                                   session.time_signature.numerator,
                                   session.time_signature.denominator,
                                   session.time_signature.ticks_per_quarter_note)
        midi_file.addTempo(track, time, session.tempo_bpm)

    @staticmethod
    def distribute_midi_notes(samples: [Sample]):
        result = {}
        low_notes = list(reversed([35, 36, 41]))
        mid_notes = list(reversed([38, 39, 40]))
        high_notes = list(reversed([42, 44, 46, 53]))

        for s in samples:
            if s.spectral_position == SpectralPositions.low:
                result[s.name] = low_notes.pop() if low_notes else s.spectral_position + 30
            elif s.spectral_position == SpectralPositions.mid:
                result[s.name] = mid_notes.pop() if mid_notes else s.spectral_position + 30
            else:
                result[s.name] = high_notes.pop() if high_notes else s.spectral_position + 30

        return result


# Session exporter that exports midi using the mido module
# This one doesn't work...
class Mido_MidiFileSessionExporter:
    @staticmethod
    def distribute_midi_notes(samples: [Sample]):
        result = {}
        note = 60
        for s in samples:
            result[s.name] = note
            note += 1
        return result

    def store_session(self, session: Session, file_path: str) -> None:
        s = copy(session)
        midi = mido.MidiFile(type=1, ticks_per_beat=s.time_signature.ticks_per_quarter_note, clip=True)
        ts_message = mido.MetaMessage('time_signature',
                                      numerator=s.time_signature.numerator,
                                      denominator=s.time_signature.denominator, time=0)
        s.events.sort(key=lambda event: event.time_stamp)
        track = mido.MidiTrack()
        midi.tracks.append(track)
        track.append(ts_message)
        notes = self.distribute_midi_notes(s.samples)
        self.add_events_to_track(s.events, track, notes)

        midi.save(filename=file_path)

    @staticmethod
    def add_events_to_track(events, track, notes):
        for i in range(len(events)):
            if i != 0:
                track.append(mido.Message('note_on',
                                          note=notes[events[i].sample.name],
                                          time=events[i].time_stamp - events[i - 1].time_stamp))
            else:
                track.append(mido.Message('note_on', note=notes[events[i].sample.name], time=events[i].time_stamp))
            track.append(mido.Message('note_off', note=events[i].duration, time=1))


# Session exporter that exports to a json file
class JsonFileSessionExporter:
    @staticmethod
    def store_session(session: Session, file_path: str) -> None:
        dictionary = convert_session_to_dictionary(session)
        json_string = json.dumps(dictionary, indent=4)
        with open(file_path, 'w') as file:
            file.write(json_string)


# running a test for making a midi file from a session
if __name__ == '__main__':
    test_session = JsonFileSessionImporter().load_session('../../config/project.json')
    MidiUtil_MidiFileSessionExporter().store_session(test_session, '../midi/midi_test.mid')
