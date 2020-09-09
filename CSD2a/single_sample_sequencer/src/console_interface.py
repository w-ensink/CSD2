# Written by Wouter Ensink

from audio_transport import AudioTransport
from events import EventGenerator


# Manages console user interaction with the transport
class ConsoleInterface:
    def __init__(self, transport: AudioTransport, event_generator: EventGenerator):
        self.transport = transport
        self.keep_running = True
        self.event_generator = event_generator
        event_time_stamps = [0, 4, 8, 12]
        self.transport.set_events(self.event_generator.generate_events(event_time_stamps))

    def run(self):
        while self.keep_running:
            command = input('-> ')
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
            else:
                print('invalid command')
