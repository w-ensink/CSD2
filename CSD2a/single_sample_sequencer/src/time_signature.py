# Written by Wouter Ensink

from unittest import TestCase


# This class resembles a time signature and the calculations related to it
class TimeSignature:
    def __init__(self, numerator, denumerator, ticks_per_quarter_note):
        self.numerator = numerator
        self.denumerator = denumerator
        self.ticks_per_quarter_note = ticks_per_quarter_note

    def is_tick_end_of_bar(self, tick):
        num_denumerators_in_quarter_note = self.denumerator / 4
        ticks_per_denumerator = self.ticks_per_quarter_note * num_denumerators_in_quarter_note
        ticks_per_bar = ticks_per_denumerator * self.numerator
        return (tick + 1) % ticks_per_bar == 0


# Unit test to make sure the time signature class functions as intended
class TimeSignature_UnitTest(TestCase):
    def test_basic_4_4_signature(self):
        ticks_per_qn = 4
        s = TimeSignature(4, 4, ticks_per_qn)
        self.assertTrue(s.is_tick_end_of_bar(15), 'tick 15 should be the last tick of a bar')
        self.assertFalse(s.is_tick_end_of_bar(16), 'tick 16 is start of new bar (ticks start at 0)')
        self.assertFalse(s.is_tick_end_of_bar(0), 'doens\'t need more info')
