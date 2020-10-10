
# Written by Wouter Ensink

from core.engine import Engine
import regex
import os


# subclasses of UserCommand should implement their behaviour and information in these given functions
# and should then be registered with the main menu
class UserCommand:
    # should return whether the command the user gave mathes this one
    def matches_command(self, command: str) -> bool:
        pass

    # should return the command in a human readable string, eg: 'remove <sample_name>'
    def get_command_as_string(self) -> str:
        pass

    # should return a nice hint about what the command does
    def get_help_string(self) -> str:
        pass

    # should perform the command on the engine
    def perform(self, engine: Engine, command: str) -> None:
        pass


class StartPlayback_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*play\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'play'

    def get_help_string(self) -> str:
        return 'play (starts playback)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.sequencer.start_playback()


class StopPlayback_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*stop\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'stop'

    def get_help_string(self) -> str:
        return 'stop (stops playback)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.sequencer.stop_playback()


# finds the 3 numeric arguments (bar, beat, tick) of a set or reset command and the sample name
def parse_change_command_arguments(command: str) -> (str, int, int, int):
    # chop off the 'r' or 's'
    command = command[2:]
    args = command.split(' ')
    result = [args[0]]
    result.extend(int(i) for i in args[1:])
    return tuple(result)


class AddEvent_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^s\s\w*\s\d*\s\d*\s\d*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 's <sample_name> <bar> <beat> <tick>'

    def get_help_string(self) -> str:
        return f'{self.get_command_as_string()} (adds event with given sample at given position)'

    def perform(self, engine: Engine, command: str) -> None:
        name, bar, beat, tick = parse_change_command_arguments(command)
        engine.session_editor.add_event(name, bar, beat, tick)


class RemoveEvent_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'r <sample_name> <bar> <beat> <tick>'

    def get_help_string(self) -> str:
        return f'{self.get_command_as_string()} (removes event with given sample from given position)'

    def perform(self, engine: Engine, command: str) -> None:
        name, bar, beat, tick = parse_change_command_arguments(command)
        engine.session_editor.remove_event(name, bar, beat, tick)


class Undo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*undo\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'undo'

    def get_help_string(self) -> str:
        return 'undo (undoes last command)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.undo()


class Redo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*redo\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'redo'

    def get_help_string(self) -> str:
        return 'redo (redoes last command)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.redo()


class ClearSample_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*clear\s[a-zA-Z_]*\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'clear <sample_name>'

    def get_help_string(self) -> str:
        return 'clear <sample_name> (removes all events using given sample)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.remove_all_events_with_sample(command.strip()[6:])


class ChangeTempo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*tempo\s\d*\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_command_as_string(self) -> str:
        return 'tempo <bpm>'

    def get_help_string(self) -> str:
        return 'tempo <bpm> (changes tempo to given argument'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.change_tempo(int(command.strip()[6:]))


# -----------------------------------------------------------------------------------
# The actual User Interface put together
class ConsoleInterface:
    def __init__(self, engine: Engine):
        self.engine = engine
        self.feedback = 'Enter "help" to see what\'s possible'
        self.command_handlers = [
            StartPlayback_UserCommand(),
            StopPlayback_UserCommand(),
            ChangeTempo_UserCommand(),
            AddEvent_UserCommand(),
            RemoveEvent_UserCommand(),
            Undo_UserCommand(),
            Redo_UserCommand(),
            ClearSample_UserCommand()
        ]

    @staticmethod
    def clear():
        os.system('clear')
        print('Wouter\'s Sequence Generator Deluxe XL Max Pro Premium\n')

    def enter_menu(self):
        while True:
            self.clear()
            print(self.engine.session_editor.get_session_as_string())
            print(f'\n{self.feedback}')
            command = input('\n---> ')
            if command == 'exit':
                return
            if not self.attempt_handling_command(command):
                self.feedback = f'Error: "{command}" is not a valid command, enter "help" to see what\'s possible'
            if command == 'help':
                self.show_help()

    def attempt_handling_command(self, command) -> bool:
        for handler in self.command_handlers:
            if handler.matches_command(command):
                handler.perform(self.engine, command)
                self.feedback = 'Enter "help" to see what\'s possible'
                return True
        return False

    def show_help(self):
        self.clear()
        print('List of valid commands:\n')
        print('\n'.join(f'- {s.get_help_string()}' for s in self.command_handlers))
        input('\nPress enter to return')
        self.feedback = 'Enter "help" to see what\'s possible'

