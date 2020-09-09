# Written by Wouter Ensink

from audio_transport import AudioTransport
from events import EventGenerator
from console_interface import ConsoleInterface


# resembles the whole sequencer put together, so both ui and the audio part
class SingleSampleSequencer:
    def __init__(self, sample_filename: str):
        self.transport = AudioTransport()
        self.event_generator = EventGenerator(sample_filename)
        self.interface = ConsoleInterface(self.transport, self.event_generator)

    def run(self):
        self.interface.run()


def main():
    SingleSampleSequencer('../audio/kick.wav').run()


if __name__ == '__main__':
    main()

