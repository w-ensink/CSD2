# Written by Wouter Ensink

from transport import AudioTransport
from events import EventManager, PyDub_EventHandler
from console_interface import ConsoleInterface
from os.path import isfile
import sys
import json


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
    def __init__(self, settings):
        event_manager = EventManager(settings)
        self.transport = AudioTransport(settings=settings, event_manager=event_manager)
        self.interface = ConsoleInterface(self.transport)

    def run(self):
        self.transport.run()


def main():
    with open('../config/settings.json', 'r') as file:
        settings = json.load(file)
        SingleSampleSequencer(settings).run()

    with open('../config/settings.json', 'w') as file:
        file.write(json.dumps(settings, indent=4))


if __name__ == '__main__':
    main()

