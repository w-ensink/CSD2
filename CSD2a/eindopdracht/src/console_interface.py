# Written by Wouter Ensink

from sequencer import Sequencer
import regex
from threading import Thread


# base class for IOProviders, main reason for using this is to make testing easier
class IOProvider:
    def present_message(self, m: str) -> None:
        pass

    def present_message_and_get_answer(self, m: str) -> str:
        pass


# the type of IO Provider that should be used with console user interaction
class ConsoleIOProvider(IOProvider):
    def present_message(self, message: str) -> None:
        print(message)

    def present_message_and_get_answer(self, message: str) -> str:
        return input(message)


# Manages console user interaction with the transport
class ConsoleInterface(Thread):
    def __init__(self, sequencer: Sequencer):
        super(ConsoleInterface, self).__init__()
        self.sequencer = sequencer
        self.keep_running = True
        self.io_provider = ConsoleIOProvider()
        self.set_event_command_pattern = regex.compile(r'^s\s\w*\s\d*\s\d*\s\d*$')
        self.reset_event_command_pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')
        # regex for any valid file path to a .wav file
        self.file_name_pattern = regex.compile(r'^[a-zA-Z0-9_/.-]*\.wav')
        # regex for: load <sample_file_path> as <sample_name>
        self.load_command_pattern = regex.compile(r'load [a-zA-Z0-9_/.-]*\.wav as [a-zA-Z_]*$')
        self.start()

    def run(self) -> None:
        while self.keep_running:
            self.handle_next_command()

    def handle_next_command(self) -> None:
        command = self.io_provider.present_message_and_get_answer('-> ')
        if command == 'start':
            self.sequencer.start_playback()
        elif command == 'stop':
            self.sequencer.stop_playback()
        elif command == 'rewind':
            self.sequencer.rewind()
        elif command == 'exit':
            self.sequencer.keep_thread_active = False
            self.keep_running = False
        elif command.isnumeric():
            self.attempt_tempo_change(int(command))
        elif command == 'change':
            self.change_sequence()
        elif self.load_command_pattern.match(command):
            self.attempt_loading_sample(command)
        elif command[:6] == 'remove':
            self.attempt_removing_sample(command)
        else:
            valid_commands = ['start', 'stop', 'rewind', 'change', '<number> (changes tempo)', 'exit']
            self.present_invalid_command_message(command, valid_commands)

    def attempt_loading_sample(self, command: str) -> None:
        command = command[5:]
        start, end = self.file_name_pattern.search(command).span()
        file_name = command[start:end]
        sample_name = command[end+4:]
        self.sequencer.sample_list.add_sample(sample_name, file_name)

    def attempt_removing_sample(self, command: str) -> None:
        sample_name = command[7:]
        self.sequencer.sample_list.remove_sample(sample_name)

    def attempt_tempo_change(self, new_tempo: int) -> None:
        if new_tempo <= 0:
            return self.io_provider.present_message(f'{new_tempo} is not a positive number, tempo change ignored...')
        self.sequencer.set_tempo_bpm(new_tempo)

    def present_invalid_command_message(self, command: str, valid_commands: [str]) -> None:
        m = f'command "{command}" is invalid, try:\n' + '\n'.join(f' - {c}' for c in valid_commands)
        self.io_provider.present_message(m)

    def print_current_sequence(self) -> None:
        time_signature = self.sequencer.time_signature
        string = self.sequencer.event_list.to_string_with_time_signature(time_signature)
        self.io_provider.present_message(string)

    def change_sequence(self) -> None:
        while True:
            self.print_current_sequence()

            m = ' - set with: s <sample_name> <bar> <beat> <tick>\n' \
                ' - reset with: r <sample_name> <bar> <beat> <tick>\n' \
                ' - exit with: done\n |-> '

            command = self.io_provider.present_message_and_get_answer(m)

            if command == 'done':
                return

            elif self.set_event_command_pattern.match(command):
                self.handle_set_event_command(command)

            elif self.reset_event_command_pattern.match(command):
                self.handle_reset_event_command(command)

            else:
                self.io_provider.present_message(f'unknown command: "{command}"')

            self.sequencer.update_looping_position()

    def handle_set_event_command(self, command: str) -> None:
        name, bar, beat, tick = self.parse_change_command_arguments(command)
        time_stamp = self.sequencer.time_signature.musical_time_to_ticks(bar, beat, tick)
        self.sequencer.event_list.add_event(sample_name=name, time_stamp=time_stamp)

    def handle_reset_event_command(self, command: str) -> None:
        name, bar, beat, tick = self.parse_change_command_arguments(command)
        time_stamp = self.sequencer.time_signature.musical_time_to_ticks(bar, beat, tick)
        self.sequencer.event_list.remove_event(sample_name=name, time_stamp=time_stamp)

    # finds the 3 numeric arguments (bar, beat, tick) of a set or reset command
    @staticmethod
    def parse_change_command_arguments(command: str) -> (str, int, int, int):
        # chop off the 'r' or 's'
        command = command[2:]
        args = command.split(' ')
        result = [args[0]]
        result.extend(int(i) for i in args[1:])
        return tuple(result)
