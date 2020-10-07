# Written by Wouter Ensink

from core.samples.sample import Sample


# just a wrapper around an already existing list of samples, to provide handy
# notifications when new samples are added by the user.
# it doesn't own the list of samples, the list of samples is probably from the session object
class SampleList:
    class Listener:
        def sample_added(self, sample: Sample) -> None:
            pass

        def sample_removed(self, sample: Sample) -> None:
            pass

    def __init__(self):
        self.samples = []
        self.listeners = []

    def add_listener(self, listener: Listener) -> None:
        self.listeners.append(listener)

    def add_sample(self, sample: Sample) -> None:
        self.samples.append(sample)

        for listener in self.listeners:
            listener.sample_added(sample)

    # note that the sample object has an equality operator
    def remove_sample(self, sample: Sample) -> None:
        for s in self.samples:
            if s == sample:
                self.samples.remove(s)
                for listener in self.listeners:
                    listener.sample_removed(sample)

    def contains(self, sample_name: str) -> bool:
        for s in self.samples:
            if s.name == sample_name:
                return True
        return False

    def get_all_sample_names(self) -> [str]:
        return [s.name for s in self.samples]

    def get_filename_for_sample(self, sample_name: str) -> str:
        for s in self.samples:
            if s.name == sample_name:
                return s.file_path

