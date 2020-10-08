# Written by Wouter Ensink

# This file contains classes that can be used by the Engine to export sequences to formats like json or midi files.
# This is a big part of the extensability of the engine. Creating a new class should be fairly straight forward.


import mido
from core.session import Session
from core.utility import convert_session_to_dictionary
import json


class MidiFileSequenceExporter:
    def store_session(self, session: Session, file_path: str):
        midi = mido.MidiFile(type=1, ticks_per_beat=session.time_signature.ticks_per_quarter_note)
        ts_message = mido.MetaMessage('time_signature',
                                      numerator=session.time_signature.numerator,
                                      denumerator=session.time_signature.denominator)
        track = mido.MidiTrack()
        midi.add_track(track)
        track.append(ts_message)
        for e in session.events:
            track.append(mido.Message('note_on', note=e.midi_note,
                                      time=e.time_stamp/session.time_signature.denominator))
            track.append(mido.Message('note_off', note=e.midi_note, time=e.duration))

        midi.save(filename=file_path)


class JsonFileSessionExporter:
    def store_session(self, session: Session, file_path: str):
        dictionary = convert_session_to_dictionary(session)
        json_string = json.dumps(dictionary, indent=4)
        with open(file_path, 'w') as file:
            file.write(json_string)


# running a test for making a midi file from a session
if __name__ == '__main__':
    midi_file = mido.MidiFile(type=1, ticks_per_beat=4)
    midi_track = mido.MidiTrack()
    midi_file.tracks.append(midi_track)

    midi_track.append(mido.MetaMessage('time_signature', numerator=5, denominator=4))
    midi_track.append(mido.MetaMessage('track_name', name='my fancy track'))
    midi_track.append(mido.MetaMessage('set_tempo', tempo=129))
    midi_track.append(mido.Message('note_on', note=63, time=1))
    midi_track.append(mido.Message('note_off', note=63, time=2))
    midi_file.save(filename='../midi/midi_test.mid')
    print('done saving midi file')
