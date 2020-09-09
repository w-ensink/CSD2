# Written by Wouter Ensink

import time
from play_head import PlayHead
from time_signature import TimeSignature
from threading import Thread
from events import Event, EventHandler


class PlayStates:
    stopped, playing = 0, 1


# responsible for the state of the playback and sending the events to the handler
class AudioTransport(Thread):
    def __init__(self):
        super(AudioTransport, self).__init__()
        self.play_state = PlayStates.stopped
        self.tempo_bpm = 100
        self.ms_between_ticks = 0
        self.ticks_per_quarter_note = 4
        self.time_signature = TimeSignature(4, 4, self.ticks_per_quarter_note)
        self.playhead = PlayHead()
        self.events = []
        self.event_handler = EventHandler()
        self.recalculate_tick_time()
        self.keep_thread_active = True
        self.start()

    def start_playback(self):
        self.play_state = PlayStates.playing

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def set_events(self, events: [Event]):
        self.events = events
        # find the event with the highest time stamp, then find the next bar ending to
        # use that as the range end of the playhead, to avoid weird looping
        loop_end = self.find_highest_event_time_stamp()

        while not self.time_signature.is_tick_end_of_bar(loop_end):
            loop_end += 1

        self.playhead.set_looping(0, loop_end)

    def find_highest_event_time_stamp(self):
        highest_time = 0
        for e in self.events:
            if e.time_stamp_ticks > highest_time:
                highest_time = e.time_stamp_ticks
        return highest_time

    def run(self):
        print('Starting Transport Thread')
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                self.handle_all_events_for_playhead_position()
                self.playhead.advance_tick()
                self.wait_for_next_tick()
        print('Stopping Transport Thread')

    def handle_all_events_for_playhead_position(self):
        for event in self.events:
            if event.time_stamp_ticks == self.playhead.position_in_ticks:
                self.event_handler.handle(event)

    def wait_for_next_tick(self):
        time.sleep(self.ms_between_ticks / 1000)

    def recalculate_tick_time(self):
        num_ticks_per_minute = self.ticks_per_quarter_note * self.tempo_bpm
        ms_per_minute = 60_000
        self.ms_between_ticks = ms_per_minute / num_ticks_per_minute

    def set_tempo_bpm(self, tempo: float):
        self.tempo_bpm = tempo
        self.recalculate_tick_time()

    def rewind(self):
        self.playhead.rewind()
