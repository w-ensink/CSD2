# Written by Wouter Ensink

from core.engine import Engine
from user_interface.transport_console_interface import MainConsoleMenu, SessionChangeMenu


def main():
    engine = Engine()
    engine.import_session('../config/project.json')
    main_menu = MainConsoleMenu(engine.sequencer)
    session_change_menu = SessionChangeMenu(engine.session_editor)
    main_menu.add_sub_menu(session_change_menu)
    main_menu.enter_menu()
    engine.shut_down()


if __name__ == '__main__':
    main()

