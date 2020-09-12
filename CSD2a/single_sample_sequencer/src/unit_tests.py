# Written by Wouter Ensink
# This file contains all unit tests for the project

from console_interface import IOProvider, ConsoleInterface
from transport import PlayStates
from unittest import TestCase
import unittest
from time_signature import TimeSignature
from play_head import PlayHead
from events import Event


# The type of IO Provider that is useful for testing all functionality of the interface
class MockIOProvider(IOProvider):
    def __init__(self, answers):
        self.answers = answers
        self.answer_index = -1
        self.last_printed = ''

    def print_message(self, m):
        self.last_printed = m

    def print_and_get_answer(self, m):
        self.last_printed = m
        self.answer_index += 1
        return self.answers[self.answer_index]


# dropin replacement for the audio transport, so it doesn't play audio during the tests...
class MockTransport:
    def __init__(self):
        self.events = []
        self.play_state = PlayStates.stopped
        self.time_signature = TimeSignature(numerator=4, denumerator=4, ticks_per_quarter_note=4)
        self.play_head = PlayHead()

    def start_playback(self):
        self.play_state = PlayStates.playing

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def rewind(self):
        self.play_head.rewind()

    def set_events(self, events):
        self.events = events


class MockEventGenerator:
    @staticmethod
    def generate_events(time_stamps):
        events = []
        for t in time_stamps:
            events.append(Event(time_stamp_ticks=t, audio_file='dummy_file.lol'))
        return events


# Unit Test for Console Interface, testing the states
class ConsoleInterface_UnitTest(TestCase):
    def test_basic_transport_control(self):
        # test basic starting and stopping and rewinding of the transport
        t = MockTransport()
        g = MockEventGenerator()
        i = ConsoleInterface(transport=t, event_generator=g)
        io_provider = MockIOProvider(answers=['start', 'stop', 'start', 'rewind'])
        i.io_provider = io_provider
        i.handle_next_command()
        self.assertEqual(t.play_state, PlayStates.playing)
        i.handle_next_command()
        self.assertEqual(t.play_state, PlayStates.stopped)
        i.handle_next_command()
        self.assertEqual(t.play_state, PlayStates.playing)
        t.play_head.position_in_ticks = 100
        # next command is now 'rewind'
        i.handle_next_command()
        self.assertEqual(t.play_head.position_in_ticks, 0)
        self.assertEqual(io_provider.last_printed, '-> ')

    def test_changing_sequence(self):
        transport = MockTransport()
        g = MockEventGenerator()
        interface = ConsoleInterface(transport=transport, event_generator=g)
        # get into change mode
        io_provider = MockIOProvider(answers=['change', 'done'])
        interface.io_provider = io_provider
        interface.handle_next_command()
        # self.assertEqual(io_provider.last_printed, '')


# Basic unit test for playhead, testing the looping functionality
class PlayHead_UnitTest(TestCase):
    def test_basic_looping(self):
        p = PlayHead()
        p.set_looping(0, 4)
        self.assertEqual(p.position_in_ticks, 0)
        p.advance_tick()
        self.assertEqual(p.position_in_ticks, 1)
        p.advance_tick()
        self.assertEqual(p.position_in_ticks, 2)
        p.advance_tick()
        self.assertEqual(p.position_in_ticks, 3)
        p.advance_tick()
        self.assertEqual(p.position_in_ticks, 0)
        p.advance_tick()
        p.rewind()
        self.assertEqual(p.position_in_ticks, 0)


# Unit test to make sure the time signature class functions as intended
class TimeSignature_UnitTest(TestCase):
    def test_basic_4_4_signature(self):
        ticks_per_qn = 4
        s = TimeSignature(4, 4, ticks_per_qn)
        self.assertFalse(s.is_tick_start_of_bar(15))
        self.assertTrue(s.is_tick_start_of_bar(16), 'tick 16 is start of new bar (ticks start at 0)')
        self.assertTrue(s.is_tick_start_of_bar(0))

    def test_5_4_signature(self):
        ticks_per_qn = 8
        s = TimeSignature(5, 4, ticks_per_qn)
        self.assertTrue(s.is_tick_start_of_bar(0))
        self.assertFalse(s.is_tick_start_of_bar(39), 'first bar range == 0 ..= 5*8, which means 40 is last')

    def test_7_8_signature(self):
        ticks_per_qn = 8
        s = TimeSignature(7, 8, ticks_per_qn)
        self.assertTrue(s.is_tick_start_of_bar(0))
        self.assertFalse(s.is_tick_start_of_bar(1))
        # each 8th note has 4/8 quarter notes, so each 8th note has 4/8 * 8 ticks
        # each bar has 7 8th notes, which would mean 7 * 4/8 * 8 ticks
        expected_num_ticks_per_bar = 7 * (4 / 8) * 8
        self.assertEqual(s.get_ticks_per_bar(), expected_num_ticks_per_bar)
        self.assertTrue(s.is_tick_start_of_bar(expected_num_ticks_per_bar * 2))


# fire up all unit tests when running this script
if __name__ == '__main__':
    unittest.main()
