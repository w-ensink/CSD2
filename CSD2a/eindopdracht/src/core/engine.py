# Written by Wouter Ensink

# This file contains the engine class. This is the main component of the core.
# The UI can give it commands and ask for status about the core and the rhythm generators.

from exporters import MidiFileSequenceExporter
from generators.euclidean import EuclideanRhythmGenerator
from core.sequencer import Sequencer
from importers import JsonFileSequenceImporter
from core.events.event_handlers import SimpleAudio_EventHandler
from core.session import Session
from core.session_editor import SessionEditor


class Engine:
    def __init__(self):
        self.midi_exporter = MidiFileSequenceExporter()
        self.json_importer = JsonFileSequenceImporter()
        self.session = Session()
        self.session_editor = SessionEditor()
        self.sequencer = Sequencer()
        self.sequencer.set_event_handler(SimpleAudio_EventHandler())
        self.sequencer.load_session(self.session)

    def import_session(self, file_name):
        session = self.json_importer.load_session(file_name)
        self.session_editor.set_session(session)
        self.sequencer.load_session(session)
        self.session = session

    def export_sequence(self, file_name):
        pass

    def generate_random_sequence(self, constraints):
        pass



