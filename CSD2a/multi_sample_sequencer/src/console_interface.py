# Written by Wouter Ensink

from transport import AudioTransport
import regex
from threading import Thread


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
class ConsoleInterface(Thread):
    def __init__(self, transport: AudioTransport):
        super(ConsoleInterface, self).__init__()
        self.transport = transport
        self.keep_running = True
        self.io_provider = ConsoleIOProvider()
        self.set_event_command_pattern = regex.compile(r'^s\s\w*\s\d*\s\d*\s\d*$')
        self.reset_event_command_pattern = regex.compile(r'^r\s\w*\s\d*\s\d*\s\d*$')
        self.start()

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
        string = self.transport.event_manager.to_string_with_time_signature(time_signature)
        self.io_provider.present_message(string)

    def change_sequence(self):
        while True:
            self.print_current_sequence()
            m = ' - set with: s <sample_name> <bar> <beat> <tick>\n' \
                ' - reset with: r <sample_name> <bar> <beat> <tick>\n' \
                ' - exit with: done\n |-> '
            command = self.io_provider.present_message_and_get_answer(m)

            if command == 'done':
                return
            elif self.set_event_command_pattern.match(command):
                name, bar, beat, tick = self.parse_change_command_arguments(command)
                time_stamp = self.transport.time_signature.musical_time_to_ticks(bar, beat, tick)
                self.transport.event_manager.add_event(sample_name=name, time_stamp=time_stamp)
            elif self.reset_event_command_pattern.match(command):
                name, bar, beat, tick = self.parse_change_command_arguments(command)
                time_stamp = self.transport.time_signature.musical_time_to_ticks(bar, beat, tick)
                self.transport.event_manager.remove_event(sample_name=name, time_stamp=time_stamp)
            else:
                self.io_provider.present_message(f'unknown command: "{command}"')

            self.transport.update_looping_position()

    # finds the 3 numeric arguments (bar, beat, tick) of a set or reset command
    @staticmethod
    def parse_change_command_arguments(command):
        # chop off the 'r' or 's'
        command = command[2:]
        args = command.split(' ')
        result = [args[0]]
        result.extend(int(i) for i in args[1:])
        return tuple(result)
