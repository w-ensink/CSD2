# Written by Wouter Ensink

import simpleaudio as sa
import time
from sample_list import SampleList
from time_signature import TimeSignature


# base class for the different types of event handlers
class EventHandler:
    def handle(self, event: dict) -> None:
        pass


# simpleaudio based event handler
class SimpleAudio_EventHandler(EventHandler, SampleList.Listener):
    def __init__(self, sample_list: SampleList):
        self.sample_list = sample_list
        self.sample_list.add_listener(self)
        self.samples = {}

        for s in sample_list.samples:
            self.samples[s['sample_name']] = sa.WaveObject.from_wave_file(s['file_name'])

    def handle(self, event: dict) -> None:
        self.samples[event['sample_name']].play()

    # if a sample is added to the sample list, this handler needs to load it for playback
    def sample_added(self, sample_name: str) -> None:
        self.samples[sample_name] \
            = sa.WaveObject.from_wave_file(self.sample_list.get_filename_for_sample(sample_name))

    # if a sample is removed from the sample list, this handler can let go of the corresponding wave object
    def sample_removed(self, sample_name: str) -> None:
        del self.samples[sample_name]


# class with methods specifically for dealing with a list of events
# requires a sample list to check if events can be added and to delete all events
# that use a particular sample when the sample gets removed from the project
class EventList(SampleList.Listener):
    def __init__(self, sample_list: SampleList, state: [dict]):
        self.events = state
        self.sample_list = sample_list
        self.sample_list.add_listener(self)

    def get_time_stamps(self) -> [int]:
        return [int(e['time_stamp']) for e in self.events]

    def get_time_stamps_for_sample(self, sample_name: str) -> [int]:
        return [int(e['time_stamp']) for e in self.events if e['sample_name'] == sample_name]

    def find_highest_time_stamp(self) -> int:
        return max(self.get_time_stamps())

    def find_looping_point_for_time_signature(self, time_signature: TimeSignature) -> int:
        loop_end = self.find_highest_time_stamp()

        # if the event is at 1st tick of a new bar, you want to loop for an extra bar
        if loop_end % time_signature.get_num_ticks_per_bar() == 0:
            return loop_end + time_signature.get_num_ticks_per_bar()

        while not time_signature.is_tick_start_of_bar(loop_end):
            loop_end += 1
        return loop_end

    def get_all_events_with_time_stamp(self, time_stamp: int) -> [dict]:
        return [e for e in self.events if e['time_stamp'] == time_stamp]

    def add_event(self, sample_name: str, time_stamp: int) -> None:
        if self.sample_list.contains(sample_name):
            self.events.append({'sample_name': sample_name, 'time_stamp': int(time_stamp)})

    def remove_event(self, sample_name: str, time_stamp: int) -> None:
        for e in self.events:
            if e['time_stamp'] == time_stamp and e['sample_name'] == sample_name:
                self.events.remove(e)

    # removes all event that use a sample that's no longer in the sample_list
    def remove_all_invalid_events(self) -> None:
        valid_sample_names = self.sample_list.get_all_sample_names()
        for e in self.events:
            if not e['sample_name'] in valid_sample_names:
                self.events.remove(e)

    # when a sample gets removed from the list of samples, all events using that sample become invalid
    # and thus need to be removed
    def sample_removed(self, sample_name: str) -> None:
        self.remove_all_invalid_events()

    def handle_all_events_with_time_stamp(self, time_stamp: int, event_handler: EventHandler) -> None:
        for e in self.events:
            if e['time_stamp'] == time_stamp:
                event_handler.handle(e)

    # makes a timeline in string format for all events with this sample, based on the given time signature
    def sample_events_to_string_with_time_signature(self, sample_name: str, time_signature: TimeSignature) -> str:
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

    def to_string_with_time_signature(self, time_signature: TimeSignature) -> str:
        return '\n'.join(f'{s[:5]}'.ljust(6) + self.sample_events_to_string_with_time_signature(s, time_signature)
                         for s in self.sample_list.get_all_sample_names())
