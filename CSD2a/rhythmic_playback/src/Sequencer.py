# Written by Wouter Ensink

from SamplePlayer import *
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


# Sequencer that just sleeps the thread between events
class Sleep_Sequencer(Sequencer):
    def play_sequence(self, sequence: SequenceDescription):
        for interval in sequence.time_intervals:
            self.sample_player.trigger()
            time.sleep(interval / 1000.0)
