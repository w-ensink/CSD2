
# Written by Wouter Ensink

from core.samples.sample import Sample
from core.samples.sample_list import SampleList
from core.events.event import Event
import simpleaudio as sa


# base class for the different types of event handlers
class EventHandler:
    def handle(self, event: Event) -> None:
        pass


# simpleaudio based event handler
class SimpleAudio_EventHandler(EventHandler, SampleList.Listener):
    def __init__(self, sample_list: SampleList):
        self.sample_list = sample_list
        self.sample_list.add_listener(self)
        self.wave_objects = {}

        for s in sample_list.samples:
            self.wave_objects[s.name] = sa.WaveObject.from_wave_file(s.file_path)

    def handle(self, event: Event) -> None:
        self.wave_objects[event.sample.name].play()

    # if a sample is added to the sample list, this handler needs to load it for playback
    def sample_added(self, sample: Sample) -> None:
        self.wave_objects[sample.name] = sa.WaveObject.from_wave_file(sample.file_path)

    # if a sample is removed from the sample list, this handler can let go of the corresponding wave object
    def sample_removed(self, sample: Sample) -> None:
        del self.wave_objects[sample.name]
