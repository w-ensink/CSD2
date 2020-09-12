# Written by Wouter Ensink

import simpleaudio as sa


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


class EventList:
    def __init__(self, events: [Event]):
        self.events = events

    def get_time_stamps(self):
        return [e.time_stamp_ticks for e in self.events]

    def find_highest_time_stamp(self):
        highest_time = 0
        for e in self.events:
            highest_time = max(e.time_stamp_ticks, highest_time)
        return highest_time

    def find_looping_point_for_time_signature(self, time_signature):
        loop_end = self.find_highest_time_stamp()
        while not time_signature.is_tick_start_of_bar(loop_end):
            loop_end += 1
        return loop_end

    def get_all_events_with_time_stamp(self, time_stamp):
        events = []
        for e in self.events:
            if e.time_stamp_ticks == time_stamp:
                events.append(e)
        return events


# generates events with the given audio file and time stamps
class EventGenerator:
    def __init__(self, audio_file):
        self.audio_file = audio_file

    def generate_events(self, time_stamps_ticks: [int]) -> EventList:
        events = []
        audio_file = sa.WaveObject.from_wave_file(self.audio_file)

        for time_stamp in time_stamps_ticks:
            events.append(Event(time_stamp_ticks=time_stamp, audio_file=audio_file))

        return EventList(events)
