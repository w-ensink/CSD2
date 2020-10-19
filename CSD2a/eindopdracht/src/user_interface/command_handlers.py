
# Written by Wouter Ensink

import regex
from os.path import isfile
from core.engine import Engine


# subclasses of CommandHandler should implement their behaviour and information in these given functions
# and should then be registered with the main menu
class CommandHandler:
    # should return whether the command the user gave mathes this one
    def matches_command(self, command: str) -> bool:
        pass

    # should return a nice hint about what the command does
    def get_help_string(self) -> str:
        pass

    # should perform the command on the engine and return a response string to be
    # displayed on the interface
    def perform(self, engine: Engine, command: str) -> str:
        pass


class StartPlayback_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:play)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'play (starts playback)'

    def perform(self, engine: Engine, command: str) -> str:
        if engine.sequencer.is_playing():
            return 'Play command ignored: sequencer was already playing'
        engine.sequencer.start_playback()
        return 'I started playback for you!'


class StopPlayback_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:stop)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'stop (stops playback)'

    def perform(self, engine: Engine, command: str) -> str:
        if not engine.sequencer.is_playing():
            return 'Stop command ignored: sequencer was already stopped'
        engine.sequencer.stop_playback()
        return 'Stopped playback for you'


# finds the 3 numeric arguments (bar, beat, tick) of a set or reset command and the sample name
def parse_add_remove_event_command_arguments(command: str) -> (str, int, int, int):
    # chop off the 'r' or 's'
    command = command[2:]
    args = command.split(' ')
    result = [args[0]]
    result.extend(int(i) for i in args[1:])
    return tuple(result)


class AddEvent_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:s)\s\w+\s\d+\s\d+\s\d+$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 's <sample_name> <bar> <beat> <tick> (adds event with given sample at given position)'

    def perform(self, engine: Engine, command: str) -> str:
        name, bar, beat, tick = parse_add_remove_event_command_arguments(command)
        engine.session_editor.add_event(name, bar, beat, tick)
        return f'Added event ({name} {bar}:{beat}:{tick})'


class RemoveEvent_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:r)\s\w+\s\d+\s\d+\s\d+$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'r <sample_name> <bar> <beat> <tick> (removes event with given sample from given position)'

    def perform(self, engine: Engine, command: str) -> str:
        name, bar, beat, tick = parse_add_remove_event_command_arguments(command)
        engine.session_editor.remove_event(name, bar, beat, tick)
        return f'Removed event ({name} {bar}:{beat}:{tick})'


class Undo_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:undo)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'undo (undoes last command)'

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.undo()
        return 'Undone last edit'


class Redo_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:redo)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'redo (redoes last command)'

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.redo()
        return 'Redone last undone edit'


class ClearSample_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:clear)\s[a-zA-Z_]+\s*$')
        self.clear_all_pattern = regex.compile(r'^(?i:clear)$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command) or self.clear_all_pattern.match(command)

    def get_help_string(self) -> str:
        return 'clear [sample_name] (removes all events [using given sample])'

    def perform(self, engine: Engine, command: str) -> str:
        if self.pattern.match(command):
            sample_name = command.strip()[6:]
            engine.session_editor.remove_all_events_with_sample(sample_name)
            return f'Removed all events using {sample_name} from session'
        else:
            engine.session_editor.remove_all_events()
            return 'Removed all events from session'


class ChangeTempo_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:tempo)\s\d+\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'tempo <bpm> (changes tempo to given argument'

    def perform(self, engine: Engine, command: str) -> str:
        tempo = int(command.strip()[6:])
        if tempo == 0:
            return '0 bpm is not a valid tempo, tempo change ignored'
        engine.session_editor.change_tempo(tempo)
        return f'Tempo changed to {tempo} bpm'


class SaveMidi_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:sm)\s[a-zA-Z_/.-]+\.(mid|midi)$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'sm <file_path> (saves session as midi)'

    def perform(self, engine: Engine, command: str) -> str:
        file_path = command[3:]
        engine.export_session_to_midi(file_path)
        return f'Saved session as midi to {file_path}'


class SaveJson_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:sj)\s[a-zA-Z_/.-]+\.json$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'sj <file_path> (saves session as json)'

    def perform(self, engine: Engine, command: str) -> str:
        file_path = command[3:]
        engine.export_session_to_json(file_path)
        return f'Saved session as json to {file_path}'


class LoadJson_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:lj)\s[a-zA-Z_/.-]+\.json$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'lj <file_path> (loads session from json)'

    def perform(self, engine: Engine, command: str) -> str:
        file_path = command[3:]
        if isfile(file_path):
            engine.load_session_from_json(file_path)
            return f'Loaded json session from {file_path}'
        return f'Couldn\'t load json from {file_path}, because it the file does not exits'


class ChangeTimeSignature_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:ts)\s\d+/\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.den_pattern = regex.compile(r'/\d+')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'ts <numerator>/<denominator> (sets the time signature)'

    def perform(self, engine: Engine, command: str) -> str:
        numerator, denominator = self.parse_command_arguments(command)
        if denominator not in [2, 4, 8, 16, 32]:
            return f'{denominator} is not a valid denominator, try 2, 4, 8 or 16 instead'
        if numerator <= 0:
            return f'{numerator} is not a valid numerator, try a positive number'
        engine.session_editor.change_time_signature(numerator, denominator, 4)
        return f'Changed time signature to {numerator}/{denominator}'

    def parse_command_arguments(self, command) -> (int, int):
        num = self.num_pattern.search(command).group()
        den = self.den_pattern.search(command).group()[1:]
        return int(num), int(den)


class Euclidean_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:euc)\s[a-zA-Z]+\s\d+$')
        self.id_pattern = regex.compile(r'^[a-zA-Z]+')
        self.num_pattern = regex.compile(r'\d+')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'euc <sample_name> <num_hits> (makes an euclidean distribution for given sample)'

    def perform(self, engine: Engine, command: str) -> str:
        name = self.id_pattern.search(command[4:]).group()
        arg = int(self.num_pattern.search(command).group())
        engine.session_editor.euclidean_for_sample(name, arg)
        return f'Made a euclidean distribution of {arg} for {name}'


class LoadSample_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:load)\s[a-zA-Z0-9_/.-]+\.wav\s(?i:as)\s[a-zA-Z]+$')
        self.file_path_pattern = regex.compile(r'[a-zA-Z0-9_/.-]+\.wav')

    def get_help_string(self) -> str:
        return 'load <file_path> as <sample_name> (loads sample from given file path as given sample name)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> str:
        path, name = self.parse_arguments(command)
        if not isfile(path):
            return f'hmmm, {path} doesn\'t exist :-('
        engine.session_editor.add_sample(path, name)
        return f'Loaded {name} from {path}\n' \
            + f'If you want to change its spectral position, type: "sp {name} <spectral_position>"'

    def parse_arguments(self, command: str) -> (str, str):
        path_match = self.file_path_pattern.search(command)
        path = path_match.group()
        name = command[path_match.span()[1] + 4:]
        return path, name


class RemoveSample_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:remove)\s[a-zA-Z]+$')

    def get_help_string(self) -> str:
        return 'remove <sample_name> (removes sample with given name from session)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> str:
        name = command[7:]
        engine.session_editor.remove_sample(command[7:])
        return f'Removed {name} and all its events from session'


class RotateSampleRight_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:rr)\s[a-zA-Z]+\s\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.name_pattern = regex.compile(r'[a-zA-Z]+')

    def get_help_string(self) -> str:
        return 'rr <sample_name> <amount> (rotates all events for given sample to the right)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> str:
        amount = int(self.num_pattern.search(command).group())
        name = self.name_pattern.search(command[2:]).group()
        engine.session_editor.rotate_all_events_with_sample(name, amount)
        return f'Rotated {name} {amount} to the right'


class RotateSampleLeft_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:rl)\s[a-zA-Z]+\s\d+$')
        self.num_pattern = regex.compile(r'\d+')
        self.name_pattern = regex.compile(r'[a-zA-Z]+')

    def get_help_string(self) -> str:
        return 'rl <sample_name> <amount> (rotates all events for given sample to the left)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> str:
        amount = int(self.num_pattern.search(command).group())
        name = self.name_pattern.search(command[2:]).group()
        engine.session_editor.rotate_all_events_with_sample(name, -amount)
        return f'Rotated {name} {amount} to the left'


class GenerateSequence_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:(surprise\sme|g))$')

    def get_help_string(self) -> str:
        return 'g (generates sequence)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.generate_sequence()
        return f'Generated Rhythm!'


class ChangeSpectralPositionForSample_CommandHandler(CommandHandler):
    def __init__(self):
        # sp <sample_name> <spectral_position>
        self.pattern = regex.compile(r'^(?i:sp)\s[a-zA-Z]+\s(?i:(low|mid|high))$')
        self.sample_name_pattern = regex.compile(r'[a-zA-Z]+')
        self.spectral_pattern = regex.compile(r'(?i:(low|mid|high))')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'sp <sample_name> <low|mid|high> (changes spectral position of given sample)'

    def perform(self, engine: Engine, command: str) -> str:
        sample_name = self.sample_name_pattern.search(command[3:]).group()
        spectral_position = self.spectral_pattern.search(command).group().lower()
        engine.session_editor.change_spectral_position_for_sample(sample_name, spectral_position)
        return f'Changed spectral position of {sample_name} to {spectral_position}'
