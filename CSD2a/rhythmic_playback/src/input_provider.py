# Written by Wouter Ensink


class RhythmDescription:
    def __init__(self, times: int, intervals_in_beats: [float], tempo: int):
        self.times, self.intervals_in_beats, self.tempo = times, intervals_in_beats, tempo
        if self.times != len(intervals_in_beats):
            print(f'self.times({times}) != len(lengths)({len(intervals_in_beats)})')
            assert False


# Base class(interface/protocol) for an input provider (leaves the design open for extension)
class InputProvider:
    # should return an object of type RhythmDescription
    def get_rhythm_description(self) -> RhythmDescription:
        pass

    # should return whether the user wants to play another rhythm
    def go_again(self) -> bool:
        pass


# implementation that provides the engine with user input from the console
class Console_InputProvider(InputProvider):
    def __init__(self):
        self.is_first_time = True

    def get_rhythm_description(self) -> RhythmDescription:
        times = int(self.ask_positive_numeric_input('Enter number of samples: '))
        intervals_in_beats: [float] = []

        for i in range(times):
            intervals_in_beats.append(self.ask_positive_numeric_input(f'Enter time interval {i + 1}: '))

        tempo = int(self.ask_positive_numeric_input('Enter tempo: '))
        return RhythmDescription(times=times, intervals_in_beats=intervals_in_beats, tempo=tempo)

    @staticmethod
    def ask_positive_numeric_input(message: str) -> float:
        while True:
            try:
                ans = float(input(message))
                if ans > 0:
                    return ans
                print(f'{ans} is not a positive number, please try again...')
            except ValueError:
                print('Couldn\'t parse answer as number, please try again...')

    def go_again(self) -> bool:
        if self.is_first_time:
            self.is_first_time = False
            return True

        while True:
            answer = input('Play another rhythm? (yes/no): ').lower()
            if answer == 'yes':
                return True
            if answer == 'no':
                return False
            print('Invalid input')
