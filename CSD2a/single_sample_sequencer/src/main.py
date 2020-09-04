# Written by Wouter Ensink

# Playhead
#   - current position in ppq (ticks)
# Transport
#   - start/stop
#   - tick density (smallest unit is 1/32 note -> tick density = 8)
#   - current BPM
#   - trigger all events at current playhead position
#          -> wait for tick duration, then increment playhead in ticks
# Sample Player?
# Event
#   - time stamp in ticks
#   - payload (audio sample)
# Console Input: should be able to change BPM
# idea: event recording by capturing keyboard input, quantize to 1/32 automatically

from threading import Thread
import time
import simpleaudio as sa


class EventGenerator:
    def __init__(self, audio_file):
        self.audio_file = audio_file

    def generate_events(self, time_stamps_ticks):
        events = []
        audio_file = sa.WaveObject.from_wave_file(self.audio_file)

        for time_stamp in time_stamps_ticks:
            events.append(Event(time_stamp_ticks=time_stamp, audio_file=audio_file))

        return events


class PlayStates:
    stopped, playing, recording = 0, 1, 2


class PlayHead:
    def __init__(self):
        self.position_in_ticks = 0

    def rewind(self):
        self.position_in_ticks = 0


class Event:
    def __init__(self, time_stamp_ticks, audio_file):
        self.time_stamp_ticks = time_stamp_ticks
        self.audio_file = audio_file


class EventHandler:
    @staticmethod
    def handle(event):
        event.audio_file.play()


class AudioTransport(Thread):
    def __init__(self):
        super(AudioTransport, self).__init__()
        self.play_state = PlayStates.stopped
        self.playhead = PlayHead()
        self.tempo_bpm = 100
        self.ms_between_ticks = 0
        self.pulses_per_quarter_note = 4
        self.events = []
        self.event_handler = EventHandler()
        self.recalculate_tick_time()
        self.keep_thread_active = True
        self.start()

    def start_playback(self):
        self.play_state = PlayStates.playing

    def stop_playback(self):
        self.play_state = PlayStates.stopped

    def set_events(self, events):
        self.events = events

    def run(self):
        print('Starting Transport Thread')
        while self.keep_thread_active:
            if self.play_state == PlayStates.playing:
                self.handle_all_events_for_playhead_position()
                self.playhead.position_in_ticks += 1
                self.wait_for_next_tick()
        print('Stopping Transport Thread')

    def handle_all_events_for_playhead_position(self):
        for event in self.events:
            if event.time_stamp_ticks == self.playhead.position_in_ticks:
                self.event_handler.handle(event)

    def wait_for_next_tick(self):
        time.sleep(self.ms_between_ticks / 1000)

    def recalculate_tick_time(self):
        num_ticks_per_minute = self.pulses_per_quarter_note * self.tempo_bpm
        self.ms_between_ticks = 60_000 / num_ticks_per_minute

    def set_tempo_bpm(self, tempo):
        self.tempo_bpm = tempo
        self.recalculate_tick_time()

    def rewind(self):
        self.playhead.rewind()


class ConsoleInterface:
    def __init__(self, transport: AudioTransport, event_generator: EventGenerator):
        self.transport = transport
        self.keep_running = True
        self.event_generator = event_generator
        event_time_stamps = [0, 4, 8, 12, 16, 18, 19, 20, 24, 28, 29, 31, 34, 36]
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


class SingleSampleSequencer:
    def __init__(self, sample_filename):
        self.transport = AudioTransport()
        self.event_generator = EventGenerator(sample_filename)
        self.interface = ConsoleInterface(self.transport, self.event_generator)

    def run(self):
        self.interface.run()


def main():
    SingleSampleSequencer('../audio/kick.wav').run()


if __name__ == '__main__':
    main()
