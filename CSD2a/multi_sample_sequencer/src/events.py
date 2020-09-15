# Written by Wouter Ensink

import simpleaudio as sa
import time


# Represents the playing of an audio file at a time stamp
class Event:
    def __init__(self, time_stamp_ticks: int, audio_file):
        self.time_stamp_ticks = time_stamp_ticks
        self.filename = audio_file

'''
# handles plays the content of an event
class EventHandler:
    def __init__(self, filenames):
        files = [sa.WaveObject.from_wave_file(f) for f in filenames]
        self.samples = dict(zip(filenames, files))

    def handle(self, event):
        self.samples[event.filename].play()
'''


class EventHandler:
    def __init__(self):
        self.file = sa.WaveObject.from_wave_file('../audio/kick.wav')

    def handle(self, e):
        self.file.play()


class EventManager:
    def __init__(self, events: [Event]):
        self.events = events

    def get_time_stamps(self):
        return [int(e.time_stamp_ticks) for e in self.events]

    def find_highest_time_stamp(self):
        highest_time = 0
        for e in self.events:
            highest_time = max(e.time_stamp_ticks, highest_time)
        return highest_time

    def find_looping_point_for_time_signature(self, time_signature):
        loop_end = self.find_highest_time_stamp()

        # if the event is at 1st tick of a new bar, you want to loop for an extra bar
        if loop_end % time_signature.get_num_ticks_per_bar() == 0:
            return loop_end + time_signature.get_num_ticks_per_bar()

        while not time_signature.is_tick_start_of_bar(loop_end):
            loop_end += 1
        return loop_end

    def get_all_events_with_time_stamp(self, time_stamp):
        return [e for e in self.events if e.time_stamp_ticks == time_stamp]

    def handle_all_events_with_time_stamp(self, time_stamp, handler):
        for e in self.events:
            if e.time_stamp_ticks == time_stamp:
                t = time.time()
                handler.handle(e)
                dt = time.time() - t
                print(f'handling {dt * 1000} ms')

    def to_string_with_time_signature(self, time_signature):
        num_ticks = int(self.find_looping_point_for_time_signature(time_signature))
        ticks_per_denum = int(time_signature.ticks_per_denumerator)
        ticks_per_bar = int(time_signature.get_num_ticks_per_bar())
        num_bars = int(num_ticks / ticks_per_bar)

        seq = [False] * num_ticks

        for i in self.get_time_stamps():
            seq[i] = True

        string = '|'

        for bar in range(num_bars):
            for beat in range(time_signature.numerator):
                for tick in range(ticks_per_denum):
                    if seq[bar * ticks_per_bar + ticks_per_denum * beat + tick]:
                        string += 'x'
                    else:
                        string += '.'
                string += ' '
            string = string[:-1] + '|'

        return string


# generates events with the given audio file and time stamps
class EventGenerator:
    def __init__(self, audio_file):
        self.audio_file = audio_file

    def generate_events(self, time_stamps_ticks: [int]) -> EventManager:
        audio_file = sa.WaveObject.from_wave_file(self.audio_file)
        return EventManager([Event(ts, audio_file) for ts in time_stamps_ticks])
