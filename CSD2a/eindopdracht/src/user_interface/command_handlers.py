
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
        engine.sequencer.stop_playback()
        return 'I stopped playback for you'


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
        return 'I added the event you wanted, I always knew something was missing there'


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
        return 'I removed the event you wanted, I always knew it was not supposed to be there'


class Undo_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:undo)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'undo (undoes last command)'

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.undo()
        return 'I undid what I just did, hope you\'re happy now'


class Redo_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^\s*(?i:redo)\s*$')

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command)

    def get_help_string(self) -> str:
        return 'redo (redoes last command)'

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.redo()
        return 'I redid all the work I had just undone for you, make up your mind!'


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
            return f'I removed all events for {sample_name}'
        else:
            engine.session_editor.remove_all_events()
            return 'I removed all events, now you can try to actually make something'


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
            return '0bpm is not a valid tempo, tempo change ignored :-('
        engine.session_editor.change_tempo(tempo)
        return f'tempo changed to {tempo}, that should do it!'


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
        return f'I saved your beat as midi to {file_path}, please don\'t leave me :-('


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
        return f'I save your session as json to {file_path}, never forget to get back to it!'


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
            return f'I loaded a json session from {file_path}, what a beat!'
        return f'I couldn\'t load json from {file_path}, because it doesn\'t exits :-('


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
            return f'{denominator} is not a valid denominator, try 2, 4, 8, 16 or 32 instead ;-)'
        if numerator <= 0:
            return f'{numerator} is not a valid numerator, try positive numbers ;-)'
        engine.session_editor.change_time_signature(numerator, denominator, 4)
        return f'I changed your time signature to {numerator}/{denominator}, good choice!'

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
        return f'Made a euclidean distribution of {arg} for {name}, enjoy!'


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
        return f'I loaded {name} from {path} :-)\n' \
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
        return f'Removed {name} from your session, I didn\'t like it either'


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
        return f'I rotated {name} {amount} to the right. Groovy, isn\'t it?'


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
        return f'I rotated {name} {amount} to the left, that was a good move!'


class GenerateSequence_CommandHandler(CommandHandler):
    def __init__(self):
        self.pattern = regex.compile(r'^(?i:surprise\sme)$')

    def get_help_string(self) -> str:
        return 'g (generates sequence)'

    def matches_command(self, command: str) -> bool:
        return self.pattern.match(command) or command == 'g' or command == 'G'

    def perform(self, engine: Engine, command: str) -> str:
        engine.session_editor.generate_sequence()
        return f'I tried my best, I hope you like it!'


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
        return f'Deep down I always knew {sample_name} was supposed to be {spectral_position}'
