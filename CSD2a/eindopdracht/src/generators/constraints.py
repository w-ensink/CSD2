# Written by Wouter Ensink

from core.sample import Sample
from core.time_signature import TimeSignature


# represents a set of constraints for the generation of a sequence,
# like the time signature, the number of bars, the samples that can be used etc...
# the generator is then free to make any rhythm it wants within these constraints
class SequenceGenerationConstraints:
    def __init__(self, samples: [Sample], time_signature: TimeSignature, num_bars: int):
        self.samples = samples
        self.time_signature = time_signature
        self.num_bars = num_bars
