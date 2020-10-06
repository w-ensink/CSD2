# Written by Wouter Ensink

from sequencer import Sequencer
from console_interface import ConsoleInterface
import json


# resembles the whole sequencer put together, so both ui and the audio part
class MultiSampleSequencer:
    def __init__(self, settings: dict):
        self.transport = Sequencer(state=settings)
        self.interface = ConsoleInterface(self.transport)

    def run(self) -> None:
        self.transport.run()


def main() -> None:
    # load the settings json file to use as a project in the sequencer
    with open('../config/settings.json', 'r') as file:
        settings = json.load(file)

    MultiSampleSequencer(settings).run()

    # save the changes made to the settings file (things like adding/removing samples and events)
    with open('../config/settings.json', 'w') as file:
        file.write(json.dumps(settings, indent=4))


if __name__ == '__main__':
    main()

