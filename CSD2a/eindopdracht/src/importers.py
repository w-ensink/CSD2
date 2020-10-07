# Written by Wouter Ensink

from core.session import Session
from core.sample import Sample, SpectralPositions
from core.event import Event
from core.time_signature import TimeSignature
from core.utility import convert_dictionary_to_session
import json


class JsonFileSessionImporter:
    def __init__(self):
        pass

    def load_session(self, file_path) -> Session:
        with open(file_path, 'r') as file:
            dictionary = json.loads(file.read())
            return convert_dictionary_to_session(dictionary)


# this will just pretend to import a session
class DummyImporter:
    def load_session(self, file_path) -> Session:
        kick = Sample(name='kick', path='../audio/kick.wav', spectral_position=SpectralPositions.low)
        perc = Sample(name='perc', path='../audio/perc.wav', spectral_position=SpectralPositions.mid)
        hat = Sample(name='hat', path='../audio/hat.wav', spectral_position=SpectralPositions.high)

        session = Session()
        session.add_sample(kick)
        session.add_sample(perc)
        session.add_sample(hat)

        time_signature = TimeSignature(4, 4, 4)
        session.change_time_signature(time_signature)
        session.change_tempo(120)

        for i in range(16):
            if i % 2 == 0:
                session.add_event(Event(sample=hat, time_stamp=i))
            if i % 8 == 4:
                session.add_event(Event(sample=perc, time_stamp=i))
            if i % 4 == 0:
                session.add_event(Event(sample=kick, time_stamp=i))

        return session
