# Written by Wouter Ensink

# This file contains the engine class. This is the main component of the core.
# It is supposed to be the only object to be controlled by the UI directly.
# The UI can give it commands and ask for status about the core and the rhythm generators.

from exporters import MidiFileSequenceExporter
from generators.euclidean import EuclideanRhythmGenerator
from core.sequencer import Sequencer
from importers import JsonFileSequenceImporter
from core.events.event_handlers import SimpleAudio_EventHandler


class Engine:
    def __init__(self):
        self.midi_exporter = MidiFileSequenceExporter()
        self.generator = EuclideanRhythmGenerator()
        self.sequencer = Sequencer()
        self.json_importer = JsonFileSequenceImporter()
        self.sequence = self.json_importer.load_sequence('.json')

    def export_sequence(self, file_name):
        pass

    def generate_random_sequence(self, constraints):
        new_sequence = self.generator.generate_random_sequence(constraints)



