# Written by Wouter Ensink


# This class resembles a time signature and the calculations related to it
class TimeSignature:
    def __init__(self, numerator: int = 4, denumerator: int = 4,
                 ticks_per_quarter_note: int = 4, settings: dict = None):
        self.numerator = numerator
        self.denumerator = denumerator
        self.ticks_per_quarter_note = ticks_per_quarter_note
        self.settings = settings
        self.load_state()
        self.ticks_per_denumerator = 0
        self.calculate_ticks_per_denumerator()

    def __del__(self):
        self.safe_state()

    def safe_state(self) -> None:
        if self.settings:
            self.settings['numerator'] = self.numerator
            self.settings['denumerator'] = self.denumerator
            self.settings['ticks_per_quarter_note'] = self.ticks_per_quarter_note

    def load_state(self) -> None:
        if self.settings:
            self.numerator = self.settings['numerator']
            self.denumerator = self.settings['denumerator']
            self.ticks_per_denumerator = self.settings['ticks_per_quarter_note']

    def is_tick_start_of_bar(self, tick: int) -> bool:
        return tick % self.get_num_ticks_per_bar() == 0

    def calculate_ticks_per_denumerator(self) -> None:
        num_denumerators_in_quarter_note = self.denumerator / 4
        self.ticks_per_denumerator = self.ticks_per_quarter_note / num_denumerators_in_quarter_note

    def get_num_ticks_per_bar(self) -> int:
        return self.ticks_per_denumerator * self.numerator

    # converts bar, beat and tick to ticks, according to this time signature
    def musical_time_to_ticks(self, bar: int, beat: int, tick: int) -> int:
        return self.get_num_ticks_per_bar() * bar + self.ticks_per_denumerator * beat + tick
