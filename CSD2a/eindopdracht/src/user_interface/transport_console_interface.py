
from core.sequencer import Sequencer
from core.session_editor import SessionEditor
import regex


class ConsoleMenu:
    def __init__(self):
        self.sub_menus = []

    def enter_menu(self):
        pass

    # should return the command that should enter this menu
    def get_enter_command(self):
        pass

    def add_sub_menu(self, menu):
        self.sub_menus.append(menu)

    def enter_sub_menu_if_needed(self, command: str):
        for m in self.sub_menus:
            if m.get_enter_command() == command:
                m.enter_menu()


# this is the main menu of the sequencer
class MainConsoleMenu(ConsoleMenu):
    def __init__(self, sequencer: Sequencer):
        super(MainConsoleMenu, self).__init__()
        self.sequencer = sequencer

    def enter_menu(self):
        while True:
            command = input(' -> ')
            if command == 'start':
                self.sequencer.start_playback()
            if command == 'stop':
                self.sequencer.stop_playback()
            if command == 'rewind':
                self.sequencer.rewind()
            if command == 'exit':
                break
            self.enter_sub_menu_if_needed(command)


class SessionChangeMenu(ConsoleMenu):
    def __init__(self, session_editor: SessionEditor):
        super(SessionChangeMenu, self).__init__()
        self.session_editor = session_editor
        self.set_event_command_pattern = regex.compile(r'^s\s\w*\s\d*\s\d*\s\d*$')
        self.reset_event_command_pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')

    def get_enter_command(self):
        return 'edit'

    def enter_menu(self):
        while True:
            print(self.session_editor.get_session_as_string())
            command = input(' ---> ')
            if self.set_event_command_pattern.match(command):
                self.handle_set_event_command(command)
            if self.reset_event_command_pattern.match(command):
                self.handle_reset_event_command(command)

    def handle_set_event_command(self, command: str) -> None:
        name, bar, beat, tick = self.parse_change_command_arguments(command)
        self.session_editor.add_event(name, bar, beat, tick)

    def handle_reset_event_command(self, command: str) -> None:
        name, bar, beat, tick = self.parse_change_command_arguments(command)
        self.session_editor.remove_event(name, bar, beat, tick)

    # finds the 3 numeric arguments (bar, beat, tick) of a set or reset command and the sample name
    @staticmethod
    def parse_change_command_arguments(command: str) -> (str, int, int, int):
        # chop off the 'r' or 's'
        command = command[2:]
        args = command.split(' ')
        result = [args[0]]
        result.extend(int(i) for i in args[1:])
        return tuple(result)
