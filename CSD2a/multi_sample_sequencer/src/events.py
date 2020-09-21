# Written by Wouter Ensink

import simpleaudio as sa
import time


# base class for the different types of event handlers
class EventHandler:
    def handle(self, event):
        pass


# simpleaudio based event handler
class SimpleAudio_EventHandler(EventHandler):
    def __init__(self, sample_list):
        self.samples = {}
        for s in sample_list:
            self.samples[s['sample_name']] = sa.WaveObject.from_wave_file(s['file_name'])

    def handle(self, event):
        # t = time.time()
        self.samples[event['sample_name']].play()
        # print(f'trigger time: {(time.time() - t) * 1000} ms')


class EventManager:
    def __init__(self, settings):
        self.events = settings['events']
        self.event_handler = SimpleAudio_EventHandler(settings['samples'])
        self.sample_names = [s['sample_name'] for s in settings['samples']]

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

    def has_sample_with_name(self, name):
        return name in self.sample_names

    def add_event(self, sample_name, time_stamp):
        if self.has_sample_with_name(sample_name):
            self.events.append({'sample_name': sample_name, 'time_stamp': int(time_stamp)})

    def remove_event(self, sample_name, time_stamp):
        for e in self.events:
            if e['time_stamp'] == time_stamp and e['sample_name'] == sample_name:
                self.events.remove(e)

    def handle_all_events_with_time_stamp(self, time_stamp):
        for e in self.events:
            if e['time_stamp'] == time_stamp:
                self.event_handler.handle(e)

    # makes a timeline in string format for all events with this sample, based on the given time signature
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



