# Written by Wouter Ensink


# This class resembles a time signature and the calculations related to it
class TimeSignature:
    def __init__(self, numerator: int = 4, denominator: int = 4, ticks_per_quarter_note: int = 4):
        self.numerator = numerator
        self.denominator = denominator
        self.ticks_per_quarter_note = ticks_per_quarter_note

    def is_tick_start_of_bar(self, tick: int) -> bool:
        return tick % self.get_num_ticks_per_bar() == 0

    def calculate_ticks_per_denominator(self) -> float:
        num_denominators_in_quarter_note = self.denominator / 4
        return self.ticks_per_quarter_note / num_denominators_in_quarter_note

    def get_num_ticks_per_bar(self) -> int:
        return int(self.calculate_ticks_per_denominator() * self.numerator)

    # converts bar, beat and tick to ticks, according to this time signature
    def musical_time_to_ticks(self, bar: int, beat: int, tick: int) -> int:
        return int(self.get_num_ticks_per_bar() * bar + self.calculate_ticks_per_denominator() * beat + tick)
