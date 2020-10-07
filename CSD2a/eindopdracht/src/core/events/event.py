# Written by Wouter Ensink

from core.samples.sample import Sample


# an event doesn't contain it's own wave object, because that wouldn't make the system as extendable
# Not all types of event handlers can make use of it. Instead, the event handler is listening to the
# list of samples and knows when a new sample is added, so it can load a new one when needed.
class Event:
    def __init__(self, sample: Sample, time_stamp: int):
        self.sample = sample
        self.time_stamp = time_stamp
        self.duration = 1
        self.midi_note = 60
        self.velocity = 127

    def __eq__(self, other):
        return self.sample == other.sample and self.time_stamp == other.time_stamp
