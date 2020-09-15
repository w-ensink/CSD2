# Written by Wouter Ensink

from sample_player import *
import time


class SequenceDescription:
    def __init__(self, time_intervals: [int]):
        self.time_intervals = time_intervals


# Base class for a sequencer (leaves the design open for extension)
class Sequencer:
    # sequence should be expected to be of type SequenceDescription
    def __init__(self):
        self.sample_player: SamplePlayer = SamplePlayer()

    def play_sequence(self, sequence: SequenceDescription):
        pass

    # player should be a subclass of SamplePlayer
    def set_sample_player(self, player: SamplePlayer):
        self.sample_player = player


# Sequencer that just sleeps the thread between events, not very accurate
class Sleep_Sequencer(Sequencer):
    def play_sequence(self, sequence: SequenceDescription):
        for interval in sequence.time_intervals:
            self.sample_player.trigger()
            time.sleep(interval / 1000.0)


# Sequencer based on absolute time points, more accurate
class AbsoluteTime_Sequencer(Sequencer):
    def play_sequence(self, sequence: SequenceDescription):
        relative_time_stamps = self.calculate_relative_time_stamps(sequence)
        time_stamp = relative_time_stamps.pop()
        start_time = time.time()

        while True:
            current_relative_time = time.time() - start_time

            if current_relative_time >= time_stamp:
                self.sample_player.trigger()

                if not relative_time_stamps:
                    break

                time_stamp = relative_time_stamps.pop()

    # calculates the time stamps starting from 0. (in seconds)
    @staticmethod
    def calculate_relative_time_stamps(sequence):
        current_time_stamp = 0
        absolute_time_stamps = []
        for i in sequence.time_intervals:
            absolute_time_stamps.append(current_time_stamp)
            current_time_stamp += i / 1000
        # reverse to make popping values work correctly
        absolute_time_stamps.reverse()
        return absolute_time_stamps

