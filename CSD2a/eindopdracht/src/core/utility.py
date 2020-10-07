# Written by Wouter Ensink

# this file contains some utility functions that don't really belong to any class
# but are still useful

from core.time_signature import TimeSignature
from core.events.event import Event
from core.session import Session
from core.sample import Sample
import unittest


def find_highest_time_stamp_in_event_list(events: [Event]) -> int:
    time_stamps = [e.time_stamp for e in events]
    return 0 if len(time_stamps) == 0 else max(time_stamps)


def find_looping_point_for_time_signature(time_stamp: int, time_signature: TimeSignature) -> int:
    # if the event is at 1st tick of a new bar, you want to loop for an extra bar
    if time_stamp % time_signature.get_num_ticks_per_bar() == 0:
        return time_stamp + time_signature.get_num_ticks_per_bar()

    while not time_signature.is_tick_start_of_bar(time_stamp):
        time_stamp += 1
    return time_stamp


def convert_session_to_dictionary(session: Session) -> dict:
    return {'samples': [{'name': s.name,
                         'path': s.file_path,
                         'spectral_position': s.spectral_position}
                        for s in session.samples],
            'events': [{'sample': {'name': e.sample.name,
                                   'path': e.sample.file_path,
                                   'spectral_position': e.sample.spectral_position},
                        'time_stamp': e.time_stamp,
                        'duration': e.duration,
                        'midi_note': e.midi_note,
                        'velocity': e.velocity}
                       for e in session.events],
            'time_signature': {'numerator': session.time_signature.numerator,
                               'denumerator': session.time_signature.denumerator,
                               'ticks_per_quarter_note': session.time_signature.ticks_per_quarter_note},
            'tempo': session.tempo_bpm}


def convert_dictionary_to_sample(dictionary: dict) -> Sample:
    return Sample(dictionary['name'], dictionary['path'], dictionary['spectral_position'])


def convert_dictionary_to_event(dictionary: dict) -> Event:
    return Event(sample=convert_dictionary_to_sample(dictionary['sample']),
                 time_stamp=dictionary['time_stamp'])


def convert_dictionary_to_time_signature(dictionary: dict) -> TimeSignature:
    return TimeSignature(numerator=dictionary['numerator'],
                         denumerator=dictionary['denumerator'],
                         ticks_per_quarter_note=dictionary['ticks_per_quarter_note'])


def convert_dictionary_to_session(dictionary: dict) -> Session:
    session = Session()
    session.samples.extend(convert_dictionary_to_sample(s) for s in dictionary['samples'])
    session.events.extend(convert_dictionary_to_event(e) for e in dictionary['events'])
    session.time_signature = convert_dictionary_to_time_signature(dictionary['time_signature'])
    session.tempo_bpm = dictionary['tempo']
    return session


# -------------------------------------------------------------------------------------------

class UtilityTests(unittest.TestCase):
    def test_find_looping_point_for_time_signature(self):
        time_sig = TimeSignature(numerator=5, denumerator=4, ticks_per_quarter_note=2)
        time_stamp = 31
        looping_point = find_looping_point_for_time_signature(time_stamp, time_sig)
        self.assertEqual(looping_point, 40)

        time_stamp = 0
        looping_point = find_looping_point_for_time_signature(time_stamp, time_sig)
        self.assertEqual(looping_point, 10)

    def test_find_highest_time_stamp_in_event_list(self):
        time_stamps = [1, 2, 3, 7, 5, 6, 74, 3, 5, 4]
        events = [Event(sample=None, time_stamp=t) for t in time_stamps]
        highest_time_stamp = find_highest_time_stamp_in_event_list(events)
        self.assertEqual(max(time_stamps), highest_time_stamp)


# runs the unit tests if this script is used as a standalone
if __name__ == '__main__':
    unittest.main()
