
# Written by Wouter Ensink

from core.sample import Sample
from core.event import Event
import simpleaudio as sa


# base class for the different types of event handlers
class EventHandler:
    def handle(self, event: Event) -> None:
        pass

    def add_sample(self, sample: Sample):
        pass

    def remove_sample(self, sample: Sample):
        pass

# --------------------------------------------------------------------------------


# simpleaudio based event handler
class SimpleAudio_EventHandler(EventHandler):
    def __init__(self):
        self.sample_list = []
        self.wave_objects = {}

    def handle(self, event: Event) -> None:
        self.wave_objects[event.sample.name].play()

    # should be used by the sequencer to let this object know when new samples are added to the session
    def add_sample(self, sample: Sample) -> None:
        self.wave_objects[sample.name] = sa.WaveObject.from_wave_file(sample.file_path)

    # if a sample is removed from the sample list, this handler can let go of the corresponding wave object
    def remove_sample(self, sample: Sample) -> None:
        del self.wave_objects[sample.name]
