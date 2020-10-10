# Written by Wouter Ensink

from core.engine import Engine
from user_interface.transport_console_interface import SessionChangeMenu


def main() -> None:
    engine = Engine()
    engine.load_session_from_file('../config/project.json')
    session_change_menu = SessionChangeMenu(engine)
    session_change_menu.enter_menu()
    engine.shut_down()


if __name__ == '__main__':
    main()

