# Written by Wouter Ensink

# A session resembles a project in the sequencer

from core.time_signature import TimeSignature
from core.sample import Sample, SpectralPositions
from core.event import Event
import unittest


# any changed to a session ABSOLUTELY NEEDS TO go through its methods,
# if not, the listener will not be notified, which will not make the sequencer
# update it's playback.
# reading data from the session can be done through the data fields, as long as it's reading only
class Session:
    class Listener:
        def sample_added_to_session(self, sample: Sample, session) -> None:
            pass

        def event_added_to_session(self, event: Event, session) -> None:
            pass

        def sample_removed_from_session(self, sample: Sample, session) -> None:
            pass

        def event_removed_from_session(self, event: Event, session) -> None:
            pass

        def time_signature_changed(self, time_signature: TimeSignature, session) -> None:
            pass

        def tempo_changed(self, tempo_bpm: float, session) -> None:
            pass

    def __init__(self):
        self.time_signature = TimeSignature()
        self.events = []
        self.samples = []
        self.tempo_bpm = 120
        self.listeners = []

    def contains_sample(self, sample: Sample) -> bool:
        return sample in self.samples

    def contains_event(self, event: Event) -> bool:
        return event in self.events

    def add_listener(self, listener: Listener) -> None:
        self.listeners.append(listener)

    def remove_listener(self, listener: Listener) -> None:
        self.listeners.remove(listener)

    def add_sample(self, sample: Sample) -> None:
        if not self.contains_sample(sample):
            self.samples.append(sample)
            for listener in self.listeners:
                listener.sample_added_to_session(sample=sample, session=self)

    def remove_sample(self, sample: Sample) -> None:
        if self.contains_sample(sample):
            self.remove_all_events_using_sample(sample)
            self.samples.remove(sample)
            for listener in self.listeners:
                listener.sample_removed_from_session(sample=sample, session=self)

    def add_event(self, event: Event) -> None:
        if self.contains_sample(event.sample) and not self.contains_event(event):
            self.events.append(event)
            for listener in self.listeners:
                listener.event_added_to_session(event=event, session=self)

    # first need to make a list with items to remove, because otherwhise it is changing the list it
    # is looping over, which results in undefined behaviour
    def remove_all_events_using_sample(self, sample: Sample) -> None:
        to_remove = []
        for e in self.events:
            if e.sample == sample:
                to_remove.append(e)
        for e in to_remove:
            self.remove_event(e)

    def remove_event(self, event: Event) -> None:
        if self.contains_event(event):
            self.events.remove(event)
            for listener in self.listeners:
                listener.event_removed_from_session(event=event, session=self)

    def change_time_signature(self, time_signature: TimeSignature) -> None:
        self.time_signature = time_signature
        for listener in self.listeners:
            listener.time_signature_changed(time_signature=time_signature, session=self)

    def change_tempo(self, tempo_bpm: float) -> None:
        if tempo_bpm > 0:
            self.tempo_bpm = tempo_bpm
            for listener in self.listeners:
                listener.tempo_changed(tempo_bpm=tempo_bpm, session=self)


# ---------------------------------------------------------------------------------------------------

# Resembles a Session.Listener for testing if all update messages get send correctly
class MockSessionListener(Session.Listener):
    def __init__(self):
        self.num_samples = 0
        self.num_events = 0
        self.num_time_signature_changes = 0
        self.current_tempo = 100

    def sample_added_to_session(self, sample: Sample, session: Session) -> None:
        self.num_samples += 1

    def sample_removed_from_session(self, sample: Sample, session) -> None:
        self.num_samples -= 1

    def event_added_to_session(self, event: Event, session: Session) -> None:
        self.num_events += 1

    def event_removed_from_session(self, event: Event, session: Session) -> None:
        self.num_events -= 1

    def time_signature_changed(self, time_signature: TimeSignature, session: Session) -> None:
        self.num_time_signature_changes += 1

    def tempo_changed(self, tempo_bpm: float, session: Session) -> None:
        self.current_tempo = tempo_bpm


class SessionTest(unittest.TestCase):
    def setUp(self) -> None:
        self.session = Session()
        self.listener = MockSessionListener()
        self.session.add_listener(self.listener)

    def test_double_same_sample_adding(self):
        self.session.add_sample(Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low))
        self.session.add_sample(Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low))
        self.assertEqual(self.listener.num_samples, 1)

    def test_double_different_sample_adding(self):
        self.session.add_sample(Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low))
        self.session.add_sample(Sample(name='snare', path='snare.wav', spectral_position=SpectralPositions.mid))
        self.assertEqual(self.listener.num_samples, 2)

    def test_adding_event(self):
        s = Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low)
        e = Event(sample=s, time_stamp=0)
        self.session.add_event(e)
        # event can't be added, because the sample it uses is not in the session yet...
        self.assertEqual(self.listener.num_events, 0)

        self.session.add_sample(s)
        self.session.add_event(e)
        self.assertEqual(self.listener.num_events, 1)

        # adding the same event twice, should not result in two identical events, but just a single event
        self.session.add_event(e)
        self.assertEqual(self.listener.num_events, 1)

        # adding a different event should actually add
        e2 = Event(sample=s, time_stamp=1)
        self.session.add_event(e2)
        self.assertEqual(self.listener.num_events, 2)

    def test_sample_removing(self):
        # first test the removing of a sample on an empty session
        s = Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low)
        self.session.add_sample(s)
        self.session.remove_sample(s)
        self.assertEqual(self.listener.num_samples, 0)

    def test_event_removing(self):
        s = Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low)
        self.session.add_sample(s)
        e = Event(sample=s, time_stamp=0)
        self.session.add_event(e)
        self.assertEqual(self.listener.num_events, 1)
        self.session.remove_event(e)
        self.assertEqual(self.listener.num_events, 0)

    # test if events that use a sample are also removed when the sample is removed
    def test_sample_removing_does_remove_events(self):
        s1 = Sample(name='kick', path='kick.wav', spectral_position=SpectralPositions.low)
        s2 = Sample(name='snare', path='snare.wav', spectral_position=SpectralPositions.mid)
        self.session.add_sample(s1)
        self.session.add_sample(s2)
        self.session.add_event(Event(sample=s1, time_stamp=0))
        self.session.add_event(Event(sample=s1, time_stamp=4))
        self.session.add_event(Event(sample=s2, time_stamp=3))
        self.assertEqual(self.listener.num_events, 3)
        self.assertEqual(self.listener.num_samples, 2)

        # now when s1 gets removed, the two events that use it should also get removed...
        self.session.remove_sample(s1)
        self.assertEqual(self.listener.num_samples, 1)
        self.assertEqual(self.listener.num_events, 1)

    def test_time_signature_change(self):
        self.assertEqual(self.listener.num_time_signature_changes, 0)
        self.session.change_time_signature(TimeSignature(numerator=5, denominator=5))
        self.assertEqual(self.listener.num_time_signature_changes, 1)

    def test_tempo_change(self):
        self.assertEqual(self.listener.current_tempo, 100)
        self.session.change_tempo(120)
        self.assertEqual(self.listener.current_tempo, 120)

    # just to assert the method is on the session class
    def test_remove_listener(self):
        self.session.remove_listener(self.listener)
        # make sure a tempo change is not passed along to this listener anymore
        self.assertEqual(self.listener.current_tempo, 100)
        self.session.change_tempo(120)
        self.assertEqual(self.listener.current_tempo, 100)


# running this file as a standalone script will just run the unit tests
if __name__ == '__main__':
    unittest.main()
