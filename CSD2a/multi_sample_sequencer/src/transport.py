# Written by Wouter Ensink

import time
from playhead import PlayHead
from time_signature import TimeSignature
from threading import Thread
from events import EventHandler, EventManager
from clock import Clock


class PlayStates:
    stopped, playing = 0, 1


# responsible for the state of the playback and sending the events to the handler
class AudioTransport(Thread):
    def __init__(self):
        super(AudioTransport, self).__init__()
        self.play_state = PlayStates.stopped
        self.tempo_bpm = 100
        self.ms_between_ticks = 0
        self.time_signature = TimeSignature(numerator=4, denumerator=4, ticks_per_quarter_note=4)
        self.playhead = PlayHead()
        self.event_manager = EventManager([])
        self.event_handler = EventHandler()
        self.recalculate_tick_time()
        self.clock = Clock(tick_time_ms=self.ms_between_ticks)
        self.keep_thread_active = True
        self.tick = 0
        self.start()

    def start_playback(self):
        self.play_state = PlayStates.playing

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def set_events(self, event_list: EventManager):
        self.event_manager = event_list
        self.playhead.set_looping(0, self.event_manager.find_looping_point_for_time_signature(self.time_signature))

    def run(self):
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                t = time.time()
                self.handle_all_events_for_playhead_position()
                dt = time.time() - t
                print(f'handle events for {self.tick} took: {dt * 1000} ms')
                self.tick += 1
                self.playhead.advance_tick()
                self.clock.block_until_next_tick()

    def handle_all_events_for_playhead_position(self):
        self.event_manager.handle_all_events_with_time_stamp(self.playhead.position_in_ticks, self.event_handler)

    def wait_for_next_tick(self):
        time.sleep(self.ms_between_ticks / 1000)

    def recalculate_tick_time(self):
        num_ticks_per_minute = self.time_signature.ticks_per_quarter_note * self.tempo_bpm
        ms_per_minute = 60_000
        self.ms_between_ticks = ms_per_minute / num_ticks_per_minute

    def set_tempo_bpm(self, tempo: float):
        if tempo > 0:
            self.tempo_bpm = tempo
            self.recalculate_tick_time()
            self.clock.update_tick_time_ms(self.ms_between_ticks)

    def rewind(self):
        self.playhead.rewind()
