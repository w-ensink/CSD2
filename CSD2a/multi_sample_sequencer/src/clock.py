# Written by Wouter Ensink

import time


# supposed to be the main clock for a sequencer,
# works by supplying a tick time, then this class will be able to
# block until the next tick, regardless how long it has been since the last tick
class Clock:
    def __init__(self, tick_time_ms):
        self.current_time = 0
        self.tick_time_ms = tick_time_ms
        self.target_time = 0

    def start(self):
        self.current_time = time.time()
        self.target_time = self.current_time + self.tick_time_ms / 1000

    def update_tick_time_ms(self, tick_time):
        self.tick_time_ms = tick_time

    def block_until_next_tick(self):
        while self.current_time < self.target_time:
            self.current_time = time.time()
        self.target_time += self.tick_time_ms / 1000
