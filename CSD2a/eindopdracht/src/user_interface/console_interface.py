
# Written by Wouter Ensink

import os
import regex
from termcolor import colored
from user_interface.command_handlers import *

# Architecture for user interface:
# the user interface is constructed of one main class that contains a load of
# command handlers. The idea is that the main class dispatches the command it receives from the user
# to the command handler that says it can handle that command.
# When telling the handler to handle the command, it also passes the engine instance
# with it, so the handler can do whatever it needs to do.
# this design makes it really easy, fast and safe to add new commands to the system


# -----------------------------------------------------------------------------------
# The actual User Interface put together
class ConsoleInterface:
    def __init__(self, engine: Engine):
        self.help_pattern = regex.compile(r'^\s*(?i:help)\s*$')
        self.exit_pattern = regex.compile(r'^\s*(?i:exit)\s*$')
        self.engine = engine
        self.feedback = 'Enter "help" to see what\'s possible'
        self.command_handlers = [
            StartPlayback_CommandHandler(),
            StopPlayback_CommandHandler(),
            GenerateSequence_CommandHandler(),
            ChangeTempo_CommandHandler(),
            ChangeTimeSignature_CommandHandler(),
            LoadSample_CommandHandler(),
            RemoveSample_CommandHandler(),
            AddEvent_CommandHandler(),
            RemoveEvent_CommandHandler(),
            Undo_CommandHandler(),
            Redo_CommandHandler(),
            ClearSample_CommandHandler(),
            SaveMidi_CommandHandler(),
            SaveJson_CommandHandler(),
            LoadJson_CommandHandler(),
            Euclidean_CommandHandler(),
            RotateSampleRight_CommandHandler(),
            RotateSampleLeft_CommandHandler(),
            ChangeSpectralPositionForSample_CommandHandler()
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

    def start_interface(self):
        while True:
            self.clear()
            print(f'{self.header}\n{self.name}\n')
            play_state = 'playing' if self.engine.sequencer.is_playing() else 'stopped'
            print(f'play state: {play_state}')
            print(self.engine.session_editor.get_session_as_string())
            print(f'\n{self.feedback}')
            command = input('\n---> ')
            self.attempt_handling_command(command)

            if self.exit_pattern.match(command):
                return
            if self.help_pattern.match(command):
                self.show_help()

    def attempt_handling_command(self, command):
        for handler in self.command_handlers:
            if handler.matches_command(command):
                self.feedback = handler.perform(self.engine, command)
                return
        self.feedback = f'Error: "{command}" is not a valid command, enter "help" to see what\'s possible'

    def show_help(self):
        self.clear()
        print(f'\n{self.name}\n')
        print('List of valid commands:\n')
        print('\n'.join(f' - {s.get_help_string()}' for s in self.command_handlers) + '\n - exit (to exit)')
        input('\nPress enter to return... ')
        self.feedback = 'Enter "help" to see what\'s possible'
