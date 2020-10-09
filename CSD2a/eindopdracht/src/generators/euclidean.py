
# what does it need to do?
# - find the greatest common divisor


import unittest
from generators.constraints import SequenceGenerationConstraints
from core.session import Session
from core.sample import Sample, SpectralPositions
from core.event import Event
import random
import math


class EuclideanRhythmGenerator:
    # euclidean distribution with the bresenham algorithm
    @staticmethod
    def make_absolute_distribution(num_ticks, num_events):
        slope = num_events / num_ticks
        previous = None
        result = []

        for i in range(num_ticks):
            current = math.floor(i * slope)
            result.append(0 if current == previous else 1)
            previous = current

        return result

    def generate_random_sequence(self, constraints: SequenceGenerationConstraints) -> Session:
        session = Session()
        session.samples = constraints.samples
        num_ticks = constraints.time_signature.get_num_ticks_per_bar()

        def calc_distribution(min_, max_):
            return math.ceil(random.randint(min_, max_) / 100 * num_ticks)

        low_density = calc_distribution(10, 30)
        mid_density = calc_distribution(20, 50)
        high_density = calc_distribution(40, 60)
        low_distribution = self.make_absolute_distribution(num_ticks, low_density)
        mid_distribution = self.make_absolute_distribution(num_ticks, mid_density)
        high_distribution = self.make_absolute_distribution(num_ticks, high_density)

        def add_events_to_session(distribution, sample):
            for index, item in enumerate(distribution):
                if item == 1:
                    session.add_event(Event(sample=sample, time_stamp=index))

        assert len(constraints.samples) == 3
        for s in constraints.samples:
            if s.spectral_position == SpectralPositions.low:
                add_events_to_session(low_distribution, s)
            if s.spectral_position == SpectralPositions.mid:
                add_events_to_session(mid_distribution, s)
            if s.spectral_position == SpectralPositions.high:
                add_events_to_session(high_distribution, s)

        session.change_time_signature(constraints.time_signature)

        return session


class Euclidean_UnitTest(unittest.TestCase):
    def setUp(self) -> None:
        self.g = EuclideanRhythmGenerator()

    def test_make_distribution(self):
        # 16, 4 -> [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0]
        self.assertEqual([1, 0, 0, 0] * 4, self.g.make_absolute_distribution(num_ticks=16, num_events=4))
        # 16, 2 -> [1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
        self.assertEqual(([1] + [0] * 7) * 2, self.g.make_absolute_distribution(num_ticks=16, num_events=2))
        # 16, 8 -> [1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0]
        self.assertEqual([1, 0] * 8, self.g.make_absolute_distribution(num_ticks=16, num_events=8))
        # 16, 5 -> [1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0]
        expected = [1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0]
        self.assertEqual(expected, self.g.make_absolute_distribution(num_ticks=16, num_events=5))
        # 16, 6 -> [1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0]
        expected = [1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0]
        self.assertEqual(expected, self.g.make_absolute_distribution(num_ticks=16, num_events=6))


if __name__ == '__main__':
    unittest.main()
