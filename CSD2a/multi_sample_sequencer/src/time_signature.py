# Written by Wouter Ensink


# This class resembles a time signature and the calculations related to it
class TimeSignature:
    def __init__(self, numerator=4, denumerator=4, ticks_per_quarter_note=4, settings=None):
        self.numerator = numerator
        self.denumerator = denumerator
        self.ticks_per_quarter_note = ticks_per_quarter_note
        self.settings = settings

        if self.settings:
            self.numerator = settings['numerator']
            self.denumerator = settings['denumerator']
            self.ticks_per_denumerator = settings['ticks_per_quarter_note']

        self.ticks_per_denumerator = 0
        self.calculate_ticks_per_denumerator()

    def safe_state(self):
        if self.settings:
            self.settings['numerator'] = self.numerator
            self.settings['denumerator'] = self.denumerator
            self.settings['ticks_per_quarter_note'] = self.ticks_per_quarter_note

    def is_tick_start_of_bar(self, tick):
        return tick % self.get_num_ticks_per_bar() == 0

    def calculate_ticks_per_denumerator(self):
        num_denumerators_in_quarter_note = self.denumerator / 4
        self.ticks_per_denumerator = self.ticks_per_quarter_note / num_denumerators_in_quarter_note

    def get_num_ticks_per_bar(self):
        return self.ticks_per_denumerator * self.numerator

    # converts bar, beat and tick to ticks, according to this time signature
    def musical_time_to_ticks(self, bar, beat, tick):
        return self.get_num_ticks_per_bar() * bar + self.ticks_per_denumerator * beat + tick
