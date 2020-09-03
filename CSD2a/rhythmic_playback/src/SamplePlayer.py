# Written by Wouter Ensink

import simpleaudio as sa


# Base class(interface/protocol) for a sample player (leaves the design open for extension)
class SamplePlayer:
    # trigger should trigger the sound in a non-blocking way (so not wait for it to finish)
    def trigger(self):
        pass


# simple sample player implementation using the 'simpleaudio' library
class SimpleAudio_SamplePlayer(SamplePlayer):
    def __init__(self, filename: str):
        self.audio_file = sa.WaveObject.from_wave_file(filename)

    def trigger(self):
        self.audio_file.play()
