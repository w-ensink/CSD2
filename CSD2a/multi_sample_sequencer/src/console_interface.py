# Written by Wouter Ensink

from transport import AudioTransport
from events import EventGenerator
import regex


# base class for IOProviders, main reason for using this is to make testing easier
class IOProvider:
    def present_message(self, m):
        pass

    def present_message_and_get_answer(self, m):
        pass


# the type of IO Provider that should be used with console user interaction
class ConsoleIOProvider(IOProvider):
    def present_message(self, message):
        print(message)

    def present_message_and_get_answer(self, message):
        return input(message)


# Manages console user interaction with the transport
class ConsoleInterface:
    def __init__(self, transport: AudioTransport, event_generator: EventGenerator):
        self.transport = transport
        self.keep_running = True
        self.event_generator = event_generator
        self.event_time_stamps = [0, 4, 8, 12, 17]
        self.transport.set_events(self.event_generator.generate_events(self.event_time_stamps))
        self.io_provider = ConsoleIOProvider()
        self.set_event_command_pattern = regex.compile(r'^s\s\d*\s\d*\s\d*$')
        self.reset_event_command_pattern = regex.compile(r'^r\s\d*\s\d*\s\d*$')

    def run(self):
        while self.keep_running:
            self.handle_next_command()

    def handle_next_command(self):
        command = self.io_provider.present_message_and_get_answer('-> ')
        if command == 'start':
            self.transport.start_playback()
        elif command == 'stop':
            self.transport.stop_playback()
        elif command == 'rewind':
            self.transport.rewind()
        elif command == 'exit':
            self.transport.keep_thread_active = False
            self.keep_running = False
        elif command.isnumeric():
            self.transport.set_tempo_bpm(int(command))
        elif command == 'change':
            self.change_sequence()
        else:
            self.present_invalid_command_message(command,
                                                 ['start', 'stop', 'rewind', 'change', '<number> (changes tempo)'])

    def present_invalid_command_message(self, command, valid_commands):
        m = f'command "{command}" is invalid, try:\n' + '\n'.join(f' - {c}' for c in valid_commands)
        self.io_provider.present_message(m)

    def print_current_sequence(self):
        time_signature = self.transport.time_signature
        string = self.transport.event_list.to_string_with_time_signature(time_signature)
        self.io_provider.present_message(string)

    def change_sequence(self):
        while True:
            self.print_current_sequence()
            m = ' - set with: s <bar> <beat> <tick>\n - reset with: r <bar> <beat> <tick>\n - done\n |-> '
            command = self.io_provider.present_message_and_get_answer(m)
            if command == 'done':
                return

            elif self.set_event_command_pattern.match(command):
                bar, beat, tick = self.parse_change_command_arguments(command)
                self.event_time_stamps.append(self.transport.time_signature.musical_time_to_ticks(bar, beat, tick))
            elif self.reset_event_command_pattern.match(command):
                bar, beat, tick = self.parse_change_command_arguments(command)
                self.event_time_stamps.remove(self.transport.time_signature.musical_time_to_ticks(bar, beat, tick))
            else:
                self.io_provider.present_message(f'unknown command: "{command}"')

            self.transport.set_events(self.event_generator.generate_events(self.event_time_stamps))

    # finds the 3 numeric arguments (bar, beat, tick) of a set or reset command
    @staticmethod
    def parse_change_command_arguments(command):
        # chop off the 'r' or 's'
        command = command[2:]
        # make a string array by cutting at space characters, then convert that to list, then to tuple
        return tuple([int(i) for i in command.split(' ')])
