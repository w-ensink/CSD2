# Written by Wouter Ensink
# This file contains all unit tests for the project

from console_interface import IOProvider, ConsoleInterface
from sequencer import PlayStates
from unittest import TestCase
import unittest
from time_signature import TimeSignature
from playhead import PlayHead


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
        self.playhead = PlayHead()

    def start_playback(self):
        self.play_state = PlayStates.playing

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def rewind(self):
        self.playhead.rewind()

    def set_events(self, events):
        self.event_list = events
        self.playhead.set_looping(0, self.event_list.find_looping_point_for_time_signature(self.time_signature))


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


# fire up all unit tests when running this script
if __name__ == '__main__':
    unittest.main()
