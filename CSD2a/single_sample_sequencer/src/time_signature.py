# Written by Wouter Ensink


# This class resembles a time signature and the calculations related to it
class TimeSignature:
    def __init__(self, numerator, denumerator, ticks_per_quarter_note):
        self.numerator = numerator
        self.denumerator = denumerator
        self.ticks_per_quarter_note = ticks_per_quarter_note
        self.ticks_per_denumerator = 0
        self.calculate_ticks_per_denumerator()

    def is_tick_start_of_bar(self, tick):
        return tick % self.get_ticks_per_bar() == 0

    def calculate_ticks_per_denumerator(self):
        num_denumerators_in_quarter_note = self.denumerator / 4
        self.ticks_per_denumerator = self.ticks_per_quarter_note / num_denumerators_in_quarter_note

    def get_ticks_per_bar(self):
        return self.ticks_per_denumerator * self.numerator
