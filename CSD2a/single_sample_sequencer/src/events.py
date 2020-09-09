# Written by Wouter Ensink

import simpleaudio as sa


# generates events with the given audio file and time stamps
class EventGenerator:
    def __init__(self, audio_file):
        self.audio_file = audio_file

    def generate_events(self, time_stamps_ticks: [int]):
        events = []
        audio_file = sa.WaveObject.from_wave_file(self.audio_file)

        for time_stamp in time_stamps_ticks:
            events.append(Event(time_stamp_ticks=time_stamp, audio_file=audio_file))

        return events


# Represents the playing of an audio file at a time stamp
class Event:
    def __init__(self, time_stamp_ticks: int, audio_file):
        self.time_stamp_ticks = time_stamp_ticks
        self.audio_file = audio_file


# handles plays the content of an event
class EventHandler:
    @staticmethod
    def handle(event):
        event.audio_file.play()
