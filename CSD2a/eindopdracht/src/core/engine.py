# Written by Wouter Ensink

# This file contains the engine class. This is the main component of the core.
# The UI can give it commands and ask for status about the core and the rhythm generators.

from exporters import MidiUtil_MidiFileSequenceExporter, JsonFileSessionExporter
from core.sequencer import Sequencer
from importers import JsonFileSessionImporter
from core.event_handlers import SimpleAudio_EventHandler
from core.session import Session
from core.session_editor import SessionEditor
from generators.euclidean import EuclideanRhythmGenerator


class Engine:
    def __init__(self):
        self.midi_exporter = MidiUtil_MidiFileSequenceExporter()
        self.json_importer = JsonFileSessionImporter()
        self.session = Session()
        self.session_editor = SessionEditor()
        self.sequencer = Sequencer()
        self.sequencer.set_event_handler(SimpleAudio_EventHandler())
        self.json_exporter = JsonFileSessionExporter()
        self.generator = EuclideanRhythmGenerator()

    def shut_down(self) -> None:
        self.sequencer.shut_down()

    def load_session_from_json(self, file_name: str) -> None:
        self.load_session(self.json_importer.load_session(file_name))

    def load_session(self, session: Session):
        # if the sequencer is playing, it should stop until the new sequence is fully loaded
        # because otherwise it could be sending events to a handler that doesn't know about the sample yet.
        should_pause_playback = self.sequencer.is_playing()
        if should_pause_playback:
            self.sequencer.stop_playback()

        self.session_editor.set_session(session)
        self.sequencer.load_session(session)
        self.session = session

        if should_pause_playback:
            self.sequencer.start_playback()

    def export_session_to_json(self, file_name: str) -> None:
        self.json_exporter.store_session(self.session, file_name)

    def export_session_to_midi(self, file_name: str) -> None:
        self.midi_exporter.store_session(self.session, file_name)

