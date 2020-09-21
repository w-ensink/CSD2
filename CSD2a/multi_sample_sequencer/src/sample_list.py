# Written by Wouter Ensink

class SampleList:
    class Listener:
        def sample_added(self, sample_name):
            pass

        def sample_removed(self, sample_name):
            pass

    def __init__(self, state):
        self.samples = state
        self.listeners = []

    def add_listener(self, listener: Listener):
        self.listeners.append(listener)

    def add_sample(self, sample_name, filename):
        self.samples.append({'sample_name': sample_name, 'file_name': filename})

        for listener in self.listeners:
            listener.sample_added(sample_name)

    def remove_sample(self, sample_name):
        for s in self.samples:
            if s['sample_name'] == sample_name:
                self.samples.remove(s)
                for listener in self.listeners:
                    listener.sample_removed(sample_name)

    def contains(self, sample_name):
        for s in self.samples:
            if s['sample_name'] == sample_name:
                return True
        return False

    def get_all_sample_names(self):
        return [s['sample_name'] for s in self.samples]

    def get_filename_for_sample(self, sample_name):
        for s in self.samples:
            if s['sample_name'] == sample_name:
                return s['file_name']

