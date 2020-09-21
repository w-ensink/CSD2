# Written by Wouter Ensink

import time
from playhead import PlayHead
from time_signature import TimeSignature
from events import EventManager
from clock import Clock


class PlayStates:
    stopped, playing = 0, 1


# responsible for the state of the playback and sending the events to the handler
class Sequencer:
    def __init__(self, state: dict):
        self.state = None
        self.play_state = PlayStates.stopped
        self.tempo_bpm = 100
        self.playhead = PlayHead()
        self.event_manager = None
        self.time_signature = None
        self.clock = None
        self.keep_thread_active = True
        self.load_state(state)

    def __del__(self):
        self.safe_state()

    def load_state(self, state: dict):
        self.state = state
        self.play_state = PlayStates.stopped
        self.tempo_bpm = state['tempo']
        self.time_signature = TimeSignature(settings=state['time_signature'])
        self.event_manager = EventManager(settings=state)
        self.clock = Clock(tick_time_ms=self.calculate_tick_time())
        self.keep_thread_active = True
        self.update_looping_position()

    def safe_state(self):
        self.state['tempo'] = self.tempo_bpm

    def start_playback(self):
        self.play_state = PlayStates.playing
        self.clock.start()

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def update_looping_position(self):
        self.playhead.set_looping(0, self.event_manager.find_looping_point_for_time_signature(self.time_signature))

    def run(self):
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                self.handle_all_events_for_playhead_position()
                self.playhead.advance_tick()
                self.clock.block_until_next_tick()

    def handle_all_events_for_playhead_position(self):
        self.event_manager.handle_all_events_with_time_stamp(self.playhead.position_in_ticks)

    def calculate_tick_time(self):
        num_ticks_per_minute = self.time_signature.ticks_per_quarter_note * self.tempo_bpm
        ms_per_minute = 60_000
        return ms_per_minute / num_ticks_per_minute

    def set_tempo_bpm(self, tempo: float):
        if tempo > 0:
            self.tempo_bpm = tempo
            self.clock.update_tick_time_ms(self.calculate_tick_time())

    def rewind(self):
        self.playhead.rewind()
