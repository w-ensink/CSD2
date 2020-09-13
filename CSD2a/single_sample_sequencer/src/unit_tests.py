# Written by Wouter Ensink
# This file contains all unit tests for the project

from console_interface import IOProvider, ConsoleInterface
from transport import PlayStates
from unittest import TestCase
import unittest
from time_signature import TimeSignature
from playhead import PlayHead
from events import Event, EventList, EventGenerator


# The type of IO Provider that is useful for testing all functionality of the interface
class MockIOProvider(IOProvider):
    def __init__(self, answers):
        self.answers = answers
        self.answer_index = -1
        self.last_printed = ''

    def present_message(self, m):
        self.last_printed = m

    def present_message_and_get_answer(self, m):
        self.last_printed = m
        self.answer_index += 1
        return self.answers[self.answer_index]


# dropin replacement for the audio transport, so it doesn't play audio during the tests...
class MockTransport:
    def __init__(self):
        self.event_list = None
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
        self.event_list = events
        self.play_head.set_looping(0, self.event_list.find_looping_point_for_time_signature(self.time_signature))


class MockEventGenerator:
    @staticmethod
    def generate_events(time_stamps):
        return EventList([Event(t, 'dummy.lol') for t in time_stamps])


# Unit Test for Console Interface, testing the states
class ConsoleInterface_UnitTest(TestCase):
    def test_basic_transport_control(self):
        # test basic starting and stopping and rewinding of the transport
        transport = MockTransport()
        event_generator = MockEventGenerator()
        interface = ConsoleInterface(transport=transport, event_generator=event_generator)
        io_provider = MockIOProvider(answers=['start', 'stop', 'start', 'rewind'])
        interface.io_provider = io_provider
        interface.handle_next_command()
        self.assertEqual(transport.play_state, PlayStates.playing)
        interface.handle_next_command()
        self.assertEqual(transport.play_state, PlayStates.stopped)
        interface.handle_next_command()
        self.assertEqual(transport.play_state, PlayStates.playing)
        transport.play_head.position_in_ticks = 100
        # next command is now 'rewind'
        interface.handle_next_command()
        self.assertEqual(transport.play_head.position_in_ticks, 0)
        self.assertEqual(io_provider.last_printed, '-> ')

    def test_changing_sequence(self):
        transport = MockTransport()
        g = MockEventGenerator()
        interface = ConsoleInterface(transport=transport, event_generator=g)
        # get into change mode
        io_provider = MockIOProvider(answers=['change', 'done'])
        interface.io_provider = io_provider
        interface.handle_next_command()
        interface.event_time_stamps = [0]
        interface.transport.set_events(interface.event_generator.generate_events(interface.event_time_stamps))
        self.assertEqual(transport.play_head.range_end, 16)
        # self.assertEqual(io_provider.last_printed, '')

    def test_regex_set_reset(self):
        transport = MockTransport()
        g = MockEventGenerator()
        interface = ConsoleInterface(transport=transport, event_generator=g)
        self.assertTrue(interface.set_event_command_pattern.match('s 0 0 0'))
        self.assertFalse(interface.set_event_command_pattern.match('s 00 0'))
        self.assertTrue(interface.reset_event_command_pattern.match('r 10 100 01'))
        self.assertFalse(interface.reset_event_command_pattern.match('r 000'))


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
        self.assertFalse(s.is_tick_start_of_bar(39), 'first bar range == 0 ..< 5*8, which means 40 start of new')
        self.assertTrue(s.is_tick_start_of_bar(40))

    def test_7_8_signature(self):
        ticks_per_qn = 8
        s = TimeSignature(7, 8, ticks_per_qn)
        self.assertTrue(s.is_tick_start_of_bar(0))
        self.assertFalse(s.is_tick_start_of_bar(1))
        # each 8th note has 4/8 quarter notes, so each 8th note has 4/8 * 8 ticks
        # each bar has 7 8th notes, which would mean 7 * 4/8 * 8 ticks
        expected_num_ticks_per_bar = 7 * (4 / 8) * 8
        self.assertEqual(s.get_num_ticks_per_bar(), expected_num_ticks_per_bar)
        self.assertTrue(s.is_tick_start_of_bar(expected_num_ticks_per_bar * 2))

    def test_musical_time_to_ticks(self):
        s = TimeSignature(numerator=5, denumerator=4, ticks_per_quarter_note=8)
        bar, beat, tick = 2, 3, 4
        expected = bar * (5 * 8) + beat * 8 + tick
        self.assertEqual(expected, s.musical_time_to_ticks(bar, beat, tick))


class EventList_UnitTest(TestCase):
    def test_find_looping_point_for_0_time_stamp(self):
        time_signature = TimeSignature(numerator=4, denumerator=4, ticks_per_quarter_note=4)
        event_time_stamps = [0]
        event_list: EventList = MockEventGenerator().generate_events(event_time_stamps)
        looping_point = event_list.find_looping_point_for_time_signature(time_signature)
        self.assertEqual(time_signature.get_num_ticks_per_bar(), looping_point)

    def test_find_highest_time_stamp(self):
        event_time_stamps = [0, 2, 5, 100]
        event_list: EventList = MockEventGenerator().generate_events(event_time_stamps)
        self.assertEqual(event_list.find_highest_time_stamp(), 100)

    def test_to_string(self):
        time_signature = TimeSignature(numerator=5, denumerator=4, ticks_per_quarter_note=8)
        event_time_stamps = [0, 8, 16, 24, 32]
        event_list: EventList = MockEventGenerator().generate_events(event_time_stamps)
        expected_string = '|x....... x....... x....... x....... x.......|'
        actual_string = event_list.to_string_with_time_signature(time_signature)
        self.assertEqual(expected_string, actual_string)

    def test_to_string_multi_bar(self):
        time_signature = TimeSignature(numerator=5, denumerator=4, ticks_per_quarter_note=2)
        event_time_stamps = [0, 2, 4, 6, 8, 10, 13]
        event_list = MockEventGenerator().generate_events(event_time_stamps)
        expected_string = '|x. x. x. x. x.|x. .x .. .. ..|'
        actual_string = event_list.to_string_with_time_signature(time_signature)
        self.assertEqual(expected_string, actual_string)


# fire up all unit tests when running this script
if __name__ == '__main__':
    unittest.main()
