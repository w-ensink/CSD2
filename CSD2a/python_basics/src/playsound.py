# Written by Wouter Ensink

import simpleaudio as sa
import sys
import regex


file_extension_pattern = regex.compile('*.[a-zA-Z0-9]$')


def extract_file_extension(filename: str) -> str:
    start_index, end_index = file_extension_pattern.find(filename)
    return filename[start_index : end_index]


def exit_with_message(message: str):
    print(message)
    exit(-1)


# gets the first commandline argument
def get_filename() -> str:
    if len(sys.argv) <= 1:
        exit_with_message('Please give file (.wav) as command line argument')

    filename = sys.argv[1].strip()
    file_extension = extract_file_extension(filename)

    if file_extension != '.wav':
        exit_with_message(f'Argument doesn\'t have a valid file extension, expected \'.wav\'')

    return filename


# plays given sound file a given number of times and waits for it to finish
def play_sound(file, num_times: int = 1):
    for _ in range(num_times):
        file.play().wait_done()


if __name__ == '__main__':
    num_times_to_play = int(input('How often would you like to play the sound?:\n'))
    sample_filename = get_filename()
    audio_file = sa.WaveObject.from_wave_file(sample_filename)
    play_sound(file=audio_file, num_times=num_times_to_play)




