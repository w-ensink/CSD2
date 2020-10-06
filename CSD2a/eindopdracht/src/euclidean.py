

# what does it need to do?
# - find the greatest common divisor


import unittest
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
