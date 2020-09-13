# Written by Wouter Ensink


# This class resembles the current position in the playback
# Also responsible for the looping
class PlayHead:
    def __init__(self):
        self.position_in_ticks = 0
        self.range_start = 0
        self.range_end = -1

    def rewind(self):
        self.position_in_ticks = 0

    def set_looping(self, range_start, range_end):
        self.range_start = range_start
        self.range_end = range_end

    def stop_looping(self):
        self.range_end = -1

    def is_looping(self):
        return self.range_end > 0

    def advance_tick(self):
        self.position_in_ticks += 1

        if self.is_looping():
            if self.position_in_ticks >= self.range_end:
                self.position_in_ticks = self.range_start
