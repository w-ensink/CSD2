
from core.sequencer import Sequencer
from core.session_editor import SessionEditor
from core.engine import Engine
from generators.constraints import SequenceGenerationConstraints
import regex
import os


class ConsoleMenu:
    def __init__(self, name):
        self.sub_menus = []
        self.name = name

    def clear(self):
        os.system('clear')
        print(f'Menu: {self.name}')

    def enter_menu(self) -> None:
        pass

    # should return the command that should enter this menu
    def get_enter_command(self) -> str:
        pass

    def add_sub_menu(self, menu) -> None:
        self.sub_menus.append(menu)

    def enter_sub_menu_if_needed(self, command: str) -> None:
        for m in self.sub_menus:
            if m.get_enter_command() == command:
                m.enter_menu()


# this is the menu for changing the session
# you can load/remove samples, add/remove events, set the time signature and change the tempo
class SessionChangeMenu(ConsoleMenu):
    def __init__(self, engine: Engine):
        super().__init__('Edit Sequence')
        self.session_editor = engine.session_editor
        self.engine = engine
        # regex for: s <sample_name> <bar> <beat> <tick>
        self.set_event_command_pattern = regex.compile(r'^s\s\w*\s\d*\s\d*\s\d*$')
        # regex for: r <sample_name> <bar> <beat> <tick>
        self.reset_event_command_pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')
        # regex for: tempo <tempo_bpm>
        self.tempo_change_command_pattern = regex.compile(r'^tempo\s*\d*$')

    def get_enter_command(self) -> str:
        return 'edit'

    def enter_menu(self) -> None:
        while True:
            self.clear()
            print(self.session_editor.get_session_as_string())
            command = input(' ---> ')
            if self.set_event_command_pattern.match(command):
                self.handle_set_event_command(command)
            if self.reset_event_command_pattern.match(command):
                self.handle_reset_event_command(command)
            if command == 'gen':
                self.engine.generate_random_sequence(None)
            if command == 'undo':
                self.session_editor.undo()
            if command == 'redo':
                self.session_editor.redo()
            if command == 'clear kick':
                self.session_editor.remove_all_events_with_sample('kick')
            if command == 'exit':
                return

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

