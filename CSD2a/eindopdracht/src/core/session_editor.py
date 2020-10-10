# Written by Wouter Ensink

# this class should be the only one actually in charge of editing the session
# this way the editing user interface only needs a reference to this class
# keeps all the details of the engine out of the way

from core.session import Session
from core.event import Event
from core.utility import session_to_formatted_string
from core.sample import Sample
from copy import copy


# Base class for any session edit action that should be undoable.
class UndoableSessionEdit:
    def perform(self, session: Session) -> None:
        pass

    def undo(self, session: Session) -> None:
        pass


# manages a bunch of UndoableSessionEdits in a stack to create full undo/redo functionality
class SessionUndoManager:
    def __init__(self):
        self.undo_stack = []
        self.redo_stack = []

    def perform(self, edit: UndoableSessionEdit, session: Session) -> None:
        edit.perform(session)
        self.undo_stack.append(edit)

    def undo(self, session: Session) -> None:
        # can't undo actions that are not there...
        if not self.undo_stack:
            return
        edit = self.undo_stack.pop()
        edit.undo(session)
        self.redo_stack.append(edit)

    def redo(self, session: Session) -> None:
        # can't redo actions that are not there...
        if not self.redo_stack:
            return
        edit = self.redo_stack.pop()
        edit.perform(session)
        self.undo_stack.append(edit)


class AddEvent_SessionEdit(UndoableSessionEdit):
    def __init__(self, event_to_add: Event):
        self.event = event_to_add

    def perform(self, session: Session) -> None:
        session.add_event(self.event)

    def undo(self, session: Session) -> None:
        session.remove_event(self.event)


class RemoveEvent_SessionEdit(UndoableSessionEdit):
    def __init__(self, event_to_remove: Event):
        self.event = event_to_remove

    def perform(self, session: Session) -> None:
        session.remove_event(self.event)

    def undo(self, session: Session) -> None:
        session.add_event(self.event)


# Action that removes all events with a given sample
class RemoveAllEventsWithSample_SessionEdit(UndoableSessionEdit):
    def __init__(self, sample: Sample):
        self.sample = sample
        self.removed_events = []

    def perform(self, session: Session) -> None:
        events = copy(session.events)
        for e in events:
            if e.sample == self.sample:
                self.removed_events.append(e)
                session.remove_event(e)

    def undo(self, session: Session) -> None:
        for e in self.removed_events:
            session.add_event(e)
        self.removed_events = []


# Session edit that changes the tempo
class ChangeTempo_SessionEdit(UndoableSessionEdit):
    def __init__(self, tempo: float):
        self.tempo = tempo

    def perform(self, session: Session) -> None:
        old_tempo = session.tempo_bpm
        session.change_tempo(self.tempo)
        self.tempo = old_tempo

    def undo(self, session: Session) -> None:
        self.perform(session)


# -----------------------------------------------------------------------------------------

class SessionEditor:
    def __init__(self):
        self.session = None
        self.undo_manager = SessionUndoManager()

    def set_session(self, session: Session) -> None:
        self.session = session

    def undo(self):
        self.undo_manager.undo(self.session)

    def redo(self):
        self.undo_manager.redo(self.session)

    def add_event(self, sample_name: str, bar: int, beat: int, tick: int) -> None:
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        event = Event(sample=sample, time_stamp=time_stamp)
        self.undo_manager.perform(AddEvent_SessionEdit(event), session=self.session)

    def remove_event(self, sample_name: str, bar: int, beat: int, tick: int) -> None:
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        event = Event(sample=sample, time_stamp=time_stamp)
        self.undo_manager.perform(RemoveEvent_SessionEdit(event), session=self.session)

    def remove_all_events_with_sample(self, sample_name: str):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        self.undo_manager.perform(RemoveAllEventsWithSample_SessionEdit(sample), self.session)

    def change_tempo(self, tempo: float):
        self.undo_manager.perform(ChangeTempo_SessionEdit(tempo), self.session)

    def find_sample_with_name(self, name: str) -> Sample or None:
        for s in self.session.samples:
            if s.name == name:
                return s
        return None

    def get_session_as_string(self) -> str:
        return session_to_formatted_string(self.session)

