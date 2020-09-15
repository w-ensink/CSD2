# Written by Wouter Ensink

import simpleaudio as sa
import time
import sounddevice as sd
import soundfile as sf
import pydub
from pydub.playback import play


# Represents the playing of an audio file at a time stamp
class Event:
    def __init__(self, time_stamp_ticks: int, sample_name: str):
        self.time_stamp = time_stamp_ticks
        self.sample_name = sample_name


# base class for the different types of event handlers
class EventHandler:
    def handle(self, event: Event):
        pass


# simpleaudio based event handler
class SimpleAudio_EventHandler(EventHandler):
    def __init__(self, sample_dict: [str, str]):
        filenames = sample_dict.values()
        sample_names = sample_dict.keys()
        files = [sa.WaveObject.from_wave_file(f) for f in filenames]
        self.samples = dict(zip(sample_names, files))

    def handle(self, event: Event):
        # t = time.time()
        self.samples[event['sample_name']].play()
        # print(f'trigger time: {(time.time() - t) * 1000} ms')


# handles plays the content of an event using sound file lib
class SoundFile_EventHandler(EventHandler):
    def __init__(self, sample_dict: [str, str]):
        filenames = sample_dict.values()
        sample_names = sample_dict.keys()
        files = []

        for f in filenames:
            file_data, self.sample_rate = sf.read(f, dtype='float32')
            files.append(file_data)
        self.samples = dict(zip(sample_names, files))

    def handle(self, event: Event):
        t = time.time()
        sd.play(self.samples[event.sample_name], self.sample_rate)
        print(f'trigger time: {(time.time() - t) * 1000} ms')


# pygame based event handler
class PyDub_EventHandler(EventHandler):
    def __init__(self, sample_dict: [str, str]):
        self.sound = pydub.AudioSegment.from_file('../audio/kick.wav')

    def handle(self, event: Event):
        t = time.time()
        play(self.sound)
        print(f'trigger time: {(time.time() - t) * 1000} ms')


class EventManager:
    def __init__(self, settings):
        self.events = settings['events']
        self.event_handler = SimpleAudio_EventHandler(settings['samples'])
        self.sample_names = settings['samples'].keys()

    def get_time_stamps(self):
        return [int(e['time_stamp']) for e in self.events]

    def get_time_stamps_for_sample(self, sample_name):
        return [int(e['time_stamp']) for e in self.events if e['sample_name'] == sample_name]

    def find_highest_time_stamp(self):
        return max(self.get_time_stamps())

    def find_looping_point_for_time_signature(self, time_signature):
        loop_end = self.find_highest_time_stamp()

        # if the event is at 1st tick of a new bar, you want to loop for an extra bar
        if loop_end % time_signature.get_num_ticks_per_bar() == 0:
            return loop_end + time_signature.get_num_ticks_per_bar()

        while not time_signature.is_tick_start_of_bar(loop_end):
            loop_end += 1
        return loop_end

    def get_all_events_with_time_stamp(self, time_stamp):
        return [e for e in self.events if e['time_stamp'] == time_stamp]

    def add_event(self, sample_name, time_stamp):
        self.events.append({'sample_name': sample_name, 'time_stamp': int(time_stamp)})

    def remove_event(self, sample_name, time_stamp):
        for e in self.events:
            if e['time_stamp'] == time_stamp and e['sample_name'] == sample_name:
                self.events.remove_event(e)

    def handle_all_events_with_time_stamp(self, time_stamp):
        for e in self.events:
            if e['time_stamp'] == time_stamp:
                self.event_handler.handle(e)

    def sample_events_to_string_with_time_signature(self, sample_name, time_signature):
        num_ticks = int(self.find_looping_point_for_time_signature(time_signature))
        ticks_per_denum = int(time_signature.ticks_per_denumerator)
        ticks_per_bar = int(time_signature.get_num_ticks_per_bar())
        num_bars = int(num_ticks / ticks_per_bar)

        seq = [False] * num_ticks

        for i in self.get_time_stamps_for_sample(sample_name):
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

    def to_string_with_time_signature(self, time_signature):
        return '\n'.join('{:<5}'.format(s[:5]) + self.sample_events_to_string_with_time_signature(s, time_signature)
                         for s in self.sample_names)



