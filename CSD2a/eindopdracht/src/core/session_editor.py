# Written by Wouter Ensink

# this class should be the only one actually in charge of editing the session
# this way the editing user interface only needs a reference to this class
# keeps all the details of the engine out of the way

from core.session import Session
from core.event import Event
from core.utility import session_to_formatted_string


class SessionEditor:
    def __init__(self):
        self.session = None

    def set_session(self, session: Session):
        self.session = session

    def add_event(self, sample_name, bar, beat, tick):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        self.session.add_event(Event(sample=sample, time_stamp=time_stamp))

    def remove_event(self, sample_name, bar, beat, tick):
        sample = self.find_sample_with_name(sample_name)
        if not sample:
            return
        time_stamp = self.session.time_signature.musical_time_to_ticks(bar, beat, tick)
        self.session.remove_event(Event(sample=sample, time_stamp=time_stamp))

    def find_sample_with_name(self, name):
        for s in self.session:
            if s.name == name:
                return s
        return None

    def get_session_as_string(self) -> str:
        return session_to_formatted_string(self.session)

