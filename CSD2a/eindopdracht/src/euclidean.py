

# what does it need to do?
# - find the greatest common divisor


import unittest
import math


class EuclideanRhythmGenerator:
    @staticmethod
    def find_greatest_common_divisor(first: int, second: int) -> int:
        highest = max(first, second)
        lowest = min(first, second)
        ans = highest
        while not (highest % ans == 0 and lowest % ans == 0):
            ans -= 1
        return ans

    @staticmethod
    def make_relative_distribution(num_ticks, num_events):
        min_interval = math.floor(num_ticks / num_events)
        max_interval = math.ceil(num_ticks / num_events)
        num_max_intervals = num_ticks - num_events * min_interval
        num_min_intervals = num_events - num_max_intervals
        result = []

        for i in range(num_min_intervals):
            result.append(min_interval)
        for i in range(num_max_intervals):
            result.append(max_interval)

        return result


class Euclidean_UnitTest(unittest.TestCase):
    def setUp(self) -> None:
        self.g = EuclideanRhythmGenerator()

    def test_make_distribution(self):
        self.assertEqual([4, 4, 4, 4], self.g.make_relative_distribution(num_ticks=16, num_events=4))
        self.assertEqual([8, 8], self.g.make_relative_distribution(num_ticks=16, num_events=2))
        self.assertEqual([2] * 8, self.g.make_relative_distribution(num_ticks=16, num_events=8))
        # 16, 5 -> [3, 3, 3, 3, 4]
        self.assertEqual([3, 3, 3, 3, 4], self.g.make_relative_distribution(num_ticks=16, num_events=5))
        # 16, 6 -> [3, 2, 3, 3, 2, 3]
        # self.assertEqual([3, 2, 3, 3, 2, 3], self.g.make_relative_distribution(num_ticks=16, num_events=6))

    def test_find_greatest_common_divisor(self):
        self.assertEqual(self.g.find_greatest_common_divisor(4, 2), 2)
        self.assertEqual(self.g.find_greatest_common_divisor(5, 2), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(4, 1), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(1, 2), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(4, 3), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(12, 3), 3)
        self.assertEqual(self.g.find_greatest_common_divisor(12, 7), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(7, 12), 1)
        self.assertEqual(self.g.find_greatest_common_divisor(10, 100), 10)


if __name__ == '__main__':
    unittest.main()
