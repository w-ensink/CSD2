# Written by Wouter Ensink

from core.engine import Engine
from user_interface.console_interface import ConsoleInterface


def main() -> None:
    engine = Engine()
    engine.load_session_from_json('../config/project.json')
    interface = ConsoleInterface(engine)
    # This starts the interface, which uses this thread until the exit command has been given
    # The sequencer runs in it's own thread, that has already been started, so this is actually desired
    interface.start_interface()
    # after the ui has stopped, it's time to shut down the engine (which stops the sequencer thread)
    engine.shut_down()


if __name__ == '__main__':
    main()
