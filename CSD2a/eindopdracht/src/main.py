# Written by Wouter Ensink

from core.engine import Engine
import time


def main():
    engine = Engine()
    engine.import_session('../config/project.json')
    engine.sequencer.start_playback()
    time.sleep(10)
    engine.shut_down()


if __name__ == '__main__':
    main()

