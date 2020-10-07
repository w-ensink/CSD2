# Written by Wouter Ensink

# This file contains classes that can be used by the Engine to export sequences to formats like json or midi files.
# This is a big part of the extensability of the engine. Creating a new class should be fairly straight forward.


import mido


class MidiFileSequenceExporter:
    def store_session(self, file_path):
        pass


class JsonFileSequenceExporter:
    pass

'''
midi_file = mido.MidiFile(type=1, clip=True)

track = mido.MidiTrack()

midi_file.add_track()
midi_file.tracks.append(track)

track.append(mido.Message('program_change', program=12, time=0))
track.append(mido.Message('note_on', note=60, time=0.0))
track.append(mido.Message('note_off', note=60, time=50))

midi_file.save(filename='../midi/f.mid')
'''