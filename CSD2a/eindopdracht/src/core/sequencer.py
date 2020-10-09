# Written by Wouter Ensink

import time
from core.playhead import PlayHead
from core.clock import Clock
from core.session import Session
from core.event_handlers import EventHandler
from core.event import Event
from core.sample import Sample
from core.time_signature import TimeSignature
from threading import Thread
from core.utility import find_highest_time_stamp_in_event_list, find_looping_point_for_time_signature


class PlayStates:
    stopped, playing = 0, 1


# the sequencer will run in it's own thread
# it is the only class that listens to a session, because it's the only
# one that needs to be updated on the fly while the sequence is changed by the user.
class Sequencer(Session.Listener, Thread):
    def __init__(self):
        Thread.__init__(self)
        self.session = Session()
        self.session.add_listener(self)
        self.play_state = PlayStates.stopped
        self.playhead = PlayHead()
        # dummy eventhandler, doesn't actually handle them. Inject acutual one via set_event_handler()
        self.event_handler: EventHandler = EventHandler()
        self.clock = Clock(tick_time_ms=1000)
        self.keep_thread_active = True
        self.update_looping_position()
        self.rewind()
        self.start()

    def load_session(self, session: Session) -> None:
        # get rid of the old session
        self.remove_all_session_samples_from_event_handler()
        self.session.remove_listener(self)
        # setup the new session
        self.session = session
        self.session.add_listener(self)
        self.add_all_session_samples_to_event_handler()
        self.clock.update_tick_time_ms(self.calculate_tick_time())
        self.update_looping_position()
        self.rewind()

    def shut_down(self) -> None:
        self.keep_thread_active = False

    def is_playing(self) -> bool:
        return self.play_state == PlayStates.playing

    def set_event_handler(self, event_handler: EventHandler) -> None:
        self.event_handler = event_handler
        # load all currently available samples in the event handler
        self.add_all_session_samples_to_event_handler()

    def add_all_session_samples_to_event_handler(self) -> None:
        for s in self.session.samples:
            self.event_handler.add_sample(s)

    def remove_all_session_samples_from_event_handler(self) -> None:
        for s in self.session.samples:
            self.event_handler.remove_sample(s)

    def start_playback(self) -> None:
        self.play_state = PlayStates.playing
        self.clock.start()

    def stop_playback(self) -> None:
        self.play_state = PlayStates.stopped

    def update_looping_position(self) -> None:
        highest_time_stamp = find_highest_time_stamp_in_event_list(self.session.events)
        looping_point = find_looping_point_for_time_signature(highest_time_stamp, self.session.time_signature)
        self.playhead.set_looping(0, looping_point)

    def run(self) -> None:
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                self.handle_all_events_for_playhead_position()
                self.playhead.advance_tick()
                self.clock.block_until_next_tick()
            else:
                # if the sequencer is stopped, sleep before checking if it has started to safe cpu
                time.sleep(0.01)

    def handle_all_events_for_playhead_position(self) -> None:
        for e in self.session.events:
            if e.time_stamp == self.playhead.position_in_ticks:
                self.event_handler.handle(e)

    def calculate_tick_time(self) -> float:
        num_ticks_per_minute = self.session.time_signature.ticks_per_quarter_note * self.session.tempo_bpm
        ms_per_minute = 60_000
        return ms_per_minute / num_ticks_per_minute

    def rewind(self) -> None:
        self.playhead.rewind()

    # --------------------------------------------------------------------------------------------
    # below are all the methods inherited from Session.Listener:

    # with a new tempo, the clock needs to be updated
    # keep in mind that this method is not responable for making sure the tempo is not 0bpm
    # it will assert for it to let it know if it somehow is...
    def tempo_changed(self, tempo_bpm: float, session: Session) -> None:
        assert session == self.session
        assert tempo_bpm != 0
        self.clock.update_tick_time_ms(self.calculate_tick_time())

    # with a different time signature the tick time could be different
    # and the logical looping point could have moved too
    def time_signature_changed(self, time_signature: TimeSignature, session: Session) -> None:
        assert session == self.session
        self.update_looping_position()
        self.clock.update_tick_time_ms(self.calculate_tick_time())

    # when a sample gets removed, events using that sample also get removed.
    # the removal of these events is already handled by the event_removed_from_sample()
    def sample_removed_from_session(self, sample: Sample, session: Session) -> None:
        assert session == self.session
        self.event_handler.remove_sample(sample)

    # when a sample gets added, tell the event handler about it
    def sample_added_to_session(self, sample: Sample, session: Session) -> None:
        assert session == self.session
        self.event_handler.add_sample(sample)

    # when a sample is added/removed, the loop could have changed in length -> update
    def event_added_to_session(self, event: Event, session: Session) -> None:
        assert session == self.session
        self.update_looping_position()

    # when a sample is added/removed, the loop could have changed in length -> update
    def event_removed_from_session(self, event: Event, session: Session) -> None:
        assert session == self.session
        self.update_looping_position()