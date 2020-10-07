# Written by Wouter Ensink

# represent the places where the samples fall in the spectrum
# is important information for the rhythm generators
class SpectralPositions:
    low, mid, high = 0, 1, 2


class Sample:
    def __init__(self, name: str, path: str, spectral_position: int):
        self.name = name
        self.file_path = path
        self.spectral_position = spectral_position

    # dedicated equality operator to prevent pointer comparison
    def __eq__(self, other):
        return self.name == other.name

