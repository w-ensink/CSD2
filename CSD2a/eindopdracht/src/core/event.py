# Written by Wouter Ensink

from core.sample import Sample


# an event doesn't contain it's own wave object, because that wouldn't make the system as extendable
# Not all types of event handlers can make use of it. Instead, the event handler is listening to the
# list of samples and knows when a new sample is added, so it can load a new one when needed.
class Event:
    def __init__(self, sample: Sample, time_stamp: int):
        self.sample = sample
        self.time_stamp = time_stamp
        self.duration = 0.25  # a quarter note
        self.midi_note = 60
        self.velocity = 127

    # dedicated equality operator to prevent pointer comparison
    def __eq__(self, other) -> bool:
        return self.sample == other.sample and self.time_stamp == other.time_stamp
