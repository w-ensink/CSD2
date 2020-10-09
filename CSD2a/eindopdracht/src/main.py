# Written by Wouter Ensink

from core.engine import Engine
from user_interface.transport_console_interface import MainConsoleMenu, SessionChangeMenu


def main() -> None:
    engine = Engine()
    engine.load_session_from_file('../config/project.json')
    main_menu = MainConsoleMenu(engine.sequencer)
    session_change_menu = SessionChangeMenu(engine)
    main_menu.add_sub_menu(session_change_menu)
    main_menu.enter_menu()
    engine.shut_down()


if __name__ == '__main__':
    main()

