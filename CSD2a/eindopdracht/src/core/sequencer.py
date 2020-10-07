# Written by Wouter Ensink

import time
from core.playhead import PlayHead
from core.time_signature import TimeSignature
from core.clock import Clock
from core.samples.sample_list import SampleList


class PlayStates:
    stopped, playing = 0, 1


class Sequencer:
    def __init__(self):
        self.state = None
        self.play_state = PlayStates.stopped
        self.tempo_bpm = 100
        self.playhead = PlayHead()
        self.event_list = None
        self.time_signature = None
        self.event_handler = None
        self.clock = Clock(tick_time_ms=1000)
        self.keep_thread_active = True
        self.update_looping_position()
        self.rewind()

    def set_event_handler(self, event_handler):
        self.event_handler = event_handler

    def start_playback(self) -> None:
        self.play_state = PlayStates.playing
        self.clock.start()

    def stop_playback(self) -> None:
        self.play_state = PlayStates.stopped

    def update_looping_position(self) -> None:
        self.playhead.set_looping(0, self.event_list.find_looping_point_for_time_signature(self.time_signature))

    def run(self) -> None:
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                self.handle_all_events_for_playhead_position()
                self.playhead.advance_tick()
                self.clock.block_until_next_tick()
            else:
                # if the core is stopped, sleep before checking if it has started to safe cpu
                time.sleep(0.01)

    def handle_all_events_for_playhead_position(self) -> None:
        self.event_list.handle_all_events_with_time_stamp(self.playhead.position_in_ticks, self.event_handler)

    def calculate_tick_time(self) -> float:
        num_ticks_per_minute = self.time_signature.ticks_per_quarter_note * self.tempo_bpm
        ms_per_minute = 60_000
        return ms_per_minute / num_ticks_per_minute

    def set_tempo_bpm(self, tempo: float) -> None:
        if tempo > 0:
            self.tempo_bpm = tempo
            self.clock.update_tick_time_ms(self.calculate_tick_time())

    def rewind(self) -> None:
        self.playhead.rewind()
