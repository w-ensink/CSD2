# Written by Wouter Ensink

from transport import AudioTransport
from events import EventGenerator
import os


# base class for IOProviders, main reason for using this is to make testing easier
class IOProvider:
    def print_message(self, m):
        pass

    def print_and_get_answer(self, m):
        pass


# the type of IO Provider that should be used with console user interaction
class ConsoleIOProvider(IOProvider):
    def print_message(self, message):
        print(message)

    def print_and_get_answer(self, message):
        return input(message)


# Manages console user interaction with the transport
class ConsoleInterface:
    def __init__(self, transport: AudioTransport, event_generator: EventGenerator):
        self.transport = transport
        self.keep_running = True
        self.event_generator = event_generator
        self.event_time_stamps = [0, 4, 8, 12]
        self.transport.set_events(self.event_generator.generate_events(self.event_time_stamps))
        self.io_provider = ConsoleIOProvider()

    def run(self):
        while self.keep_running:
            self.handle_next_command()

    def handle_next_command(self):
        command = self.io_provider.print_and_get_answer('-> ')
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
            self.io_provider.print_message('invalid command')

    def print_current_sequence(self):
        time_signature = self.transport.time_signature
        ticks_per_denum = int(time_signature.ticks_per_denumerator)
        seq = [False] * ticks_per_denum * time_signature.numerator

        for i in self.event_time_stamps:
            seq[i] = True

        string = '|'

        for bar in range(time_signature.numerator):
            for tick in range(ticks_per_denum):
                if seq[time_signature.numerator * bar + tick]:
                    string += 'x'
                else:
                    string += '.'
            string += '|'
        os.system('clear')
        print(string)

    def change_sequence(self):
        while True:
            self.print_current_sequence()
            m = ' - set with: s <beat> <tick>\n - reset with: r <beat> <tick>\n |-> '
            command = self.io_provider.print_and_get_answer(m)
            if command == 'done':
                return
            bar = int(command[2])
            tick = int(command[4])
            to_set = self.transport.time_signature.numerator * bar + tick
            if command[0] == 's':
                self.event_time_stamps.append(to_set)
            if command[0] == 'r':
                self.event_time_stamps.remove(to_set)

            self.transport.set_events(self.event_generator.generate_events(self.event_time_stamps))
