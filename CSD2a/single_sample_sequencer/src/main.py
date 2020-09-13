# Written by Wouter Ensink

from transport import AudioTransport
from events import EventGenerator
from console_interface import ConsoleInterface
from os.path import isfile
import sys


def exit_with_message(message: str):
    print(message)
    exit(-1)


# gets the first commandline argument, checks if the file exists and if it's a wav file
def get_filename() -> str:
    if len(sys.argv) <= 1:
        exit_with_message('Please give file (.wav) as command line argument')

    filename = sys.argv[1].strip()

    if not isfile(filename):
        exit_with_message(f'{filename} is not a file')

    if not filename.endswith('.wav'):
        exit_with_message(f'Argument doesn\'t have a valid file extension, expected \'.wav\'')

    return filename


# resembles the whole sequencer put together, so both ui and the audio part
class SingleSampleSequencer:
    def __init__(self, sample_filename: str):
        self.transport = AudioTransport()
        self.event_generator = EventGenerator(sample_filename)
        self.interface = ConsoleInterface(self.transport, self.event_generator)

    def run(self):
        self.interface.run()


def main():
    SingleSampleSequencer(get_filename()).run()


if __name__ == '__main__':
    main()

