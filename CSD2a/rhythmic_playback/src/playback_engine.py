# Written by Wouter Ensink

from input_provider import *
from sequencer import *


class RhythmToSequenceConverter:
    def convert(self, rhythm: RhythmDescription) -> SequenceDescription:
        ms: float = self.tempo_to_ms_per_quarter_note(rhythm.tempo)
        return SequenceDescription(ms * length for length in rhythm.intervals_in_beats)

    @staticmethod
    def tempo_to_ms_per_quarter_note(tempo: int) -> float:
        return 60_000.0 / float(tempo)


class RhythmicPlaybackEngine:
    def __init__(self, sample_filename: str):
        self.sequencer: Sequencer = AbsoluteTime_Sequencer()
        self.sequencer.set_sample_player(SimpleAudio_SamplePlayer(filename=sample_filename))
        self.input_provider: InputProvider = Console_InputProvider()
        self.converter = RhythmToSequenceConverter()

    def run(self):
        while self.input_provider.go_again():
            rhythm: RhythmDescription = self.input_provider.get_rhythm_description()
            sequence: SequenceDescription = self.converter.convert(rhythm)
            self.sequencer.play_sequence(sequence)
