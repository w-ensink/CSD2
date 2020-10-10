# Written by Wouter Ensink

from core.engine import Engine
from user_interface.console_interface import ConsoleInterface


def main() -> None:
    engine = Engine()
    engine.load_session_from_json('../config/project.json')
    session_change_menu = ConsoleInterface(engine)
    session_change_menu.enter_menu()
    engine.shut_down()


if __name__ == '__main__':
    main()

