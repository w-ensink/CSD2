
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

    def get_help_string(self) -> str:
        return 'play (starts playback)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.sequencer.start_playback()


class StopPlayback_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*stop\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

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

    def get_help_string(self) -> str:
        return 's <sample_name> <bar> <beat> <tick> (adds event with given sample at given position)'

    def perform(self, engine: Engine, command: str) -> None:
        name, bar, beat, tick = parse_change_command_arguments(command)
        engine.session_editor.add_event(name, bar, beat, tick)


class RemoveEvent_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'r <sample_name> <bar> <beat> <tick> (removes event with given sample from given position)'

    def perform(self, engine: Engine, command: str) -> None:
        name, bar, beat, tick = parse_change_command_arguments(command)
        engine.session_editor.remove_event(name, bar, beat, tick)


class Undo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*undo\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'undo (undoes last command)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.undo()


class Redo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*redo\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'redo (redoes last command)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.redo()


class ClearSample_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*clear\s[a-zA-Z_]*\s*$')
        self.clear_all_pattern = regex.compile(r'^clear$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command) or self.clear_all_pattern.match(command)

    def get_help_string(self) -> str:
        return 'clear [sample_name] (removes all events [using given sample])'

    def perform(self, engine: Engine, command: str) -> None:
        if self.pattern.match(command):
            engine.session_editor.remove_all_events_with_sample(command.strip()[6:])
        else:
            engine.session_editor.remove_all_events()


class ChangeTempo_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*tempo\s\d*\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'tempo <bpm> (changes tempo to given argument'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.change_tempo(int(command.strip()[6:]))


class SaveMidi_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^sm\s[a-zA-Z_/.-]+\.(mid|midi)$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'sm <file_path> (saves session as midi)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.export_session_to_midi(command[3:])


class SaveJson_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^sj\s[a-zA-Z_/.-]+\.json$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'sj <file_path> (saves session as json)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.export_session_to_json(command[3:])


class LoadJson_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^lj\s[a-zA-Z_/.-]+\.json$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'lj <file_path> (loads session from json)'

    def perform(self, engine: Engine, command: str) -> None:
        engine.load_session_from_json(command[3:])


class ChangeTimeSignature_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^ts\s\d+/\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.den_pattern = regex.compile(r'/\d+')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'ts <numerator>/<denominator> (sets the time signature)'

    def perform(self, engine: Engine, command: str) -> None:
        numerator, denominator = self.parse_command_arguments(command)
        engine.session_editor.change_time_signature(numerator, denominator, 4)

    def parse_command_arguments(self, command) -> (int, int):
        num = self.num_pattern.search(command).group()
        den = self.den_pattern.search(command).group()[1:]
        return int(num), int(den)


class Euclidean_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^euc\s[a-zA-Z]+\s\d+$')
        self.id_pattern = regex.compile(r'^[a-zA-Z]+')
        self.num_pattern = regex.compile(r'\d+')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'euc <sample_name> <num_hits> (makes an euclidean distribution for given sample)'

    def perform(self, engine: Engine, command: str) -> None:
        name = self.id_pattern.search(command[4:]).group()
        arg = int(self.num_pattern.search(command).group())
        engine.session_editor.euclidean_for_sample(name, arg)


class LoadSample_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^load\s[a-zA-Z0-9_/.-]+\.wav\sas\s[a-zA-Z]+$')
        self.file_path_pattern = regex.compile(r'[a-zA-Z0-9_/.-]+\.wav')

    def get_help_string(self) -> str:
        return 'load <file_path> as <sample_name> (loads sample from given file path as given sample name)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> None:
        path, name = self.parse_arguments(command)
        engine.session_editor.add_sample(path, name)

    def parse_arguments(self, command: str) -> (str, str):
        path_match = self.file_path_pattern.search(command)
        path = path_match.group()
        name = command[path_match.span()[1] + 4:]
        return path, name


class RemoveSample_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^remove\s[a-zA-Z]+$')

    def get_help_string(self) -> str:
        return 'remove <sample_name> (removes sample with given name from session)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.remove_sample(command[7:])


class RotateSampleRight_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^rr\s[a-zA-Z]+\s\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.name_pattern = regex.compile(r'[a-zA-Z]+')

    def get_help_string(self) -> str:
        return 'rr <sample_name> <amount> (rotates all events for given sample to the right)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> None:
        amount = int(self.num_pattern.search(command).group())
        name = self.name_pattern.search(command[2:]).group()
        engine.session_editor.rotate_all_events_with_sample(name, amount)


class RotateSampleLeft_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^rl\s[a-zA-Z]+\s\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.name_pattern = regex.compile(r'[a-zA-Z]+')

    def get_help_string(self) -> str:
        return 'rl <sample_name> <amount> (rotates all events for given sample to the left)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> None:
        amount = int(self.num_pattern.search(command).group())
        name = self.name_pattern.search(command[2:]).group()
        engine.session_editor.rotate_all_events_with_sample(name, -amount)


class GenerateSequence_UserCommand(UserCommand):
    def __init__(self):
        self.pattern = regex.compile(r'^surprise\sme$')

    def get_help_string(self) -> str:
        return 'surprise me (generates sequence)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command) or command == 'g'

    def perform(self, engine: Engine, command: str) -> None:
        engine.session_editor.generate_sequence()


# -----------------------------------------------------------------------------------
# The actual User Interface put together
class ConsoleInterface:
    def __init__(self, engine: Engine):
        self.engine = engine
        self.feedback = 'Enter "help" to see what\'s possible'
        self.command_handlers = [
            StartPlayback_UserCommand(),
            StopPlayback_UserCommand(),
            GenerateSequence_UserCommand(),
            ChangeTempo_UserCommand(),
            ChangeTimeSignature_UserCommand(),
            LoadSample_UserCommand(),
            RemoveSample_UserCommand(),
            AddEvent_UserCommand(),
            RemoveEvent_UserCommand(),
            Undo_UserCommand(),
            Redo_UserCommand(),
            ClearSample_UserCommand(),
            SaveMidi_UserCommand(),
            SaveJson_UserCommand(),
            LoadJson_UserCommand(),
            Euclidean_UserCommand(),
            RotateSampleRight_UserCommand(),
            RotateSampleLeft_UserCommand()
        ]
        self.name = 'Wouter\'s Sequence Generator Deluxe XL Max Pro Premium'
        self.header = '''
░██╗░░░░░░░██╗░█████╗░██╗░░░██╗████████╗███████╗██████╗░
░██║░░██╗░░██║██╔══██╗██║░░░██║╚══██╔══╝██╔════╝██╔══██╗
░╚██╗████╗██╔╝██║░░██║██║░░░██║░░░██║░░░█████╗░░██████╔╝
░░████╔═████║░██║░░██║██║░░░██║░░░██║░░░██╔══╝░░██╔══██╗
░░╚██╔╝░╚██╔╝░╚█████╔╝╚██████╔╝░░░██║░░░███████╗██║░░██║
░░░╚═╝░░░╚═╝░░░╚════╝░░╚═════╝░░░░╚═╝░░░╚══════╝╚═╝░░╚═╝'''

    def __del__(self):
        self.clear()
        print(self.header)
        print(f'\nThank you for using\n{self.name} :-)\n\n\n')

    @staticmethod
    def clear():
        if os.name == 'posix':
            os.system('clear')
        else:
            os.system('cls')

    def enter_menu(self):
        while True:
            self.clear()
            print(f'{self.header}\n{self.name}\n')
            print(self.engine.session_editor.get_session_as_string())
            print(f'\n{self.feedback}')
            command = input('\n---> ')
            self.attempt_handling_command(command)

            if command == 'exit':
                return
            if command == 'help':
                self.show_help()

    def attempt_handling_command(self, command):
        for handler in self.command_handlers:
            if handler.matches_command(command):
                self.feedback = 'Enter "help" to see what\'s possible'
                return handler.perform(self.engine, command)
        self.feedback = f'Error: "{command}" is not a valid command, enter "help" to see what\'s possible'

    def show_help(self):
        self.clear()
        print(f'\n{self.name}\n')
        print('List of valid commands:\n')
        print('\n'.join(f' - {s.get_help_string()}' for s in self.command_handlers) + '\n - exit (to exit)')
        input('\nPress enter to return... ')
        self.feedback = 'Enter "help" to see what\'s possible'

