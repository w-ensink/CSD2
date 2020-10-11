# Written by Wouter Ensink

# this class should be the only one actually in charge of editing the session
# this way the editing user interface only needs a reference to this class
# keeps all the details of the engine out of the way

from core.time_signature import TimeSignature
from core.session import Session
from core.event import Event
from core.utility import session_to_formatted_string, \
    find_all_events_with_sample, \
    find_looping_point_for_session, \
    wrap
from core.sample import Sample, SpectralPositions
from copy import copy
from generators.euclidean import EuclideanRhythmGenerator
import random
import math


# Base class for any session edit action that should be undoable.
class UndoableSessionEdit:
    def perform(self, session: Session) -> None:
        pass

    def undo(self, session: Session) -> None:
        pass


# manages a bunch of UndoableSessionEdits in a stack to create full undo/redo functionality
class SessionEditManager:
    def __init__(self):
        self.undo_stack = []
        self.redo_stack = []

    def perform_edit(self, edit: UndoableSessionEdit, session: Session) -> None:
        edit.perform(session)
        self.undo_stack.append(edit)

    def undo_last_edit(self, session: Session) -> None:
        # can't undo actions that are not there...
        if not self.undo_stack:
            return
        edit = self.undo_stack.pop()
        edit.undo(session)
        self.redo_stack.append(edit)

    def redo_last_undone_edit(self, session: Session) -> None:
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


class RemoveAllEvents_SessionEdit(UndoableSessionEdit):
    def __init__(self):
        self.backup = []

    def perform(self, session: Session) -> None:
        self.backup = copy(session.events)
        for e in self.backup:
            session.remove_event(e)

    def undo(self, session: Session) -> None:
        for e in self.backup:
            session.add_event(e)


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


class ChangeTimeSignature_SessionEdit(UndoableSessionEdit):
    def __init__(self, time_signature: TimeSignature):
        self.time_signature = time_signature

    def perform(self, session: Session) -> None:
        old = session.time_signature
        session.change_time_signature(self.time_signature)
        self.time_signature = old

    # undo is the same as perform
    def undo(self, session: Session) -> None:
        self.perform(session)


class EuclideanForSample_SessionEdit(UndoableSessionEdit):
    def __init__(self, num_events: int, sample: Sample):
        self.clear_edit = RemoveAllEventsWithSample_SessionEdit(sample)
        self.num_events = num_events
        self.sample = sample

    def perform(self, session: Session) -> None:
        self.clear_edit.perform(session)
        num_ticks = session.time_signature.get_num_ticks_per_bar()
        distribution = EuclideanRhythmGenerator.make_absolute_distribution(num_ticks=num_ticks,
                                                                           num_events=self.num_events)
        for index, item in enumerate(distribution):
            if item == 1:
                session.add_event(Event(sample=self.sample, time_stamp=index))

    def undo(self, session: Session) -> None:
        RemoveAllEventsWithSample_SessionEdit(self.sample).perform(session)
        self.clear_edit.undo(session)


class AddSample_SessionEdit(UndoableSessionEdit):
    def __init__(self, sample: Sample):
        self.sample = sample

    def perform(self, session: Session) -> None:
        session.add_sample(self.sample)

    def undo(self, session: Session) -> None:
        session.remove_sample(self.sample)


class RemoveSample_SessionEdit(UndoableSessionEdit):
    def __init__(self, sample: Sample):
        self.sample = sample
        self.event_backup = None

    def perform(self, session: Session) -> None:
        self.event_backup = copy(session.events)
        session.remove_sample(self.sample)

    def undo(self, session: Session) -> None:
        session.add_sample(self.sample)
        for e in self.event_backup:
            session.add_event(e)


# rotates all events that use a particular sample
class RotateSample_SessionEdit(UndoableSessionEdit):
    def __init__(self, sample: Sample, amount: int):
        self.sample = sample
        self.amount = amount

    def perform(self, session: Session) -> None:
        events = find_all_events_with_sample(session, self.sample)
        RemoveAllEventsWithSample_SessionEdit(self.sample).perform(session)
        looping_point = find_looping_point_for_session(session)

        for e in events:
            e.time_stamp = wrap(value=e.time_stamp + self.amount, wrapping_point=looping_point)
            session.add_event(e)

    def undo(self, session: Session) -> None:
        events = find_all_events_with_sample(session, self.sample)
        RemoveAllEventsWithSample_SessionEdit(self.sample).perform(session)
        looping_point = find_looping_point_for_session(session)

        for e in events:
            e.time_stamp = wrap(value=e.time_stamp - self.amount, wrapping_point=looping_point)
            session.add_event(e)


class GenerateSequence_SessionEdit(UndoableSessionEdit):
    def __init__(self):
        self.clear_edit = RemoveAllEvents_SessionEdit()

    def perform(self, session: Session) -> None:
        self.clear_edit.perform(session)
        num_ticks = session.time_signature.get_num_ticks_per_bar()
        low_dist = math.ceil(random.randint(0, num_ticks) / 4)
        mid_dist = math.ceil(random.randint(0, num_ticks) / 4)
        high_dist = math.ceil(random.randint(0, num_ticks - 4))
        for s in session.samples:
            if s.spectral_position == SpectralPositions.low:
                EuclideanForSample_SessionEdit(low_dist, s).perform(session)
            if s.spectral_position == SpectralPositions.mid:
                EuclideanForSample_SessionEdit(mid_dist, s).perform(session)
                RotateSample_SessionEdit(s, random.randint(4, num_ticks - 4)).perform(session)
            if s.spectral_position == SpectralPositions.high:
                EuclideanForSample_SessionEdit(high_dist, s).perform(session)
                RotateSample_SessionEdit(s, random.randint(0, num_ticks)).perform(session)

    def undo(self, session: Session) -> None:
        RemoveAllEvents_SessionEdit().perform(session)
        self.clear_edit.undo(session)


# -----------------------------------------------------------------------------------------

class SessionEditor:
    def __init__(self):
        self.session = None
        self.edit_manager = SessionEditManager()

    def set_session(self, session: Session) -> None:
        self.session = session

    def undo(self):
        self.edit_manager.undo_last_edit(self.session)

    def redo(self):
        self.edit_manager.redo_last_undone_edit(self.session)

    def add_sample(self, path: str, name: str):
        # can't add sample with the same name as an existing sample
        if self.find_sample_with_name(name):
            return
        sample = Sample(name, path, 0)
        self.edit_manager.perform_edit(AddSample_SessionEdit(sample), self.session)

    def remove_sample(self, name: str):
        sample = self.find_sample_with_name(name)
        if not sample:
            return
        self.edit_manager.perform_edit(RemoveSample_SessionEdit(sample), self.session)

    def add_event(self, sample_name: str, bar: int, beat: int, tick: int) -> None:
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        event = Event(sample=sample, time_stamp=time_stamp)
        self.edit_manager.perform_edit(AddEvent_SessionEdit(event), session=self.session)

    def remove_event(self, sample_name: str, bar: int, beat: int, tick: int) -> None:
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        event = Event(sample=sample, time_stamp=time_stamp)
        self.edit_manager.perform_edit(RemoveEvent_SessionEdit(event), session=self.session)

    def remove_all_events_with_sample(self, sample_name: str):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        self.edit_manager.perform_edit(RemoveAllEventsWithSample_SessionEdit(sample), self.session)

    def remove_all_events(self):
        self.edit_manager.perform_edit(RemoveAllEvents_SessionEdit(), self.session)

    def change_tempo(self, tempo: float):
        self.edit_manager.perform_edit(ChangeTempo_SessionEdit(tempo), self.session)

    def change_time_signature(self, numerator: int, denominator: int, ticks_per_quarter_note: int):
        ts = TimeSignature(numerator, denominator, ticks_per_quarter_note)
        self.edit_manager.perform_edit(ChangeTimeSignature_SessionEdit(ts), self.session)

    def euclidean_for_sample(self, sample_name: str, num_events: int):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        self.edit_manager.perform_edit(EuclideanForSample_SessionEdit(sample=sample, num_events=num_events), self.session)

    def find_sample_with_name(self, name: str) -> Sample or None:
        for s in self.session.samples:
            if s.name == name:
                return s
        return None

    def get_session_as_string(self) -> str:
        return session_to_formatted_string(self.session)

    def rotate_all_events_with_sample(self, sample_name: str, amount: int):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        self.edit_manager.perform_edit(RotateSample_SessionEdit(sample, amount), self.session)

    def generate_sequence(self):
        self.edit_manager.perform_edit(GenerateSequence_SessionEdit(), self.session)

