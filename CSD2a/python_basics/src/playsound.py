# Written by Wouter Ensink

import simpleaudio as sa
import sys
from os.path import isfile, basename


def exit_with_message(message: str):
    print(message)
    exit(-1)


# gets the first commandline argument, checks if the file exists and if it's a wav file
def get_filename() -> str:
    if len(sys.argv) <= 1:
        exit_with_message('Please give file (.wav) as command line argument')

    filename = sys.argv[1].strip()

    if not isfile(filename):
        exit_with_message(f'{filename} is not a file')

    if not filename.endswith('.wav'):
        exit_with_message(f'Argument doesn\'t have a valid file extension, expected \'.wav\'')

    return filename


# plays given sound file a given number of times and waits for it to finish (blocking)
def play_sound(filename, num_times: int = 1) -> None:
    print(f'Playing \'{basename(filename)}\' {num_times} time(s)')
    file = sa.WaveObject.from_wave_file(filename)

    for _ in range(num_times):
        file.play().wait_done()


# ask the user how often to play the sample and don't stop asking until a valid answer has been given
def ask_num_times_to_play() -> int:
    while True:
        ans = input('How often would you like to play the sound?: ')
        try:
            value = int(ans)
        except ValueError:
            print('Could not parse answer as integer value, please try again...')
            continue

        if value > 0:
            return value

        print('Can\'t play non-positive number of times, please try again...')


# ask if the user wants to play the sample again, doesn't stop until a valid answer has been given
def ask_go_again() -> bool:
    while True:
        ans = input('Want to go again? (y/n): ').lower()
        if ans == 'y':
            return True
        if ans == 'n':
            return False
        print('Invalid input, try again...')


def main() -> None:
    go_again = True
    sample_filename = get_filename()

    while go_again:
        num_times_to_play = ask_num_times_to_play()
        play_sound(filename=sample_filename, num_times=num_times_to_play)
        go_again = ask_go_again()

    print('Okay, bye')


if __name__ == '__main__':
    main()
