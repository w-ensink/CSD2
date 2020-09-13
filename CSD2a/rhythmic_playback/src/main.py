# Written by Wouter Ensink

from playback_engine import RhythmicPlaybackEngine
import sys
from os.path import isfile


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


def main() -> None:
    filename = get_filename()
    RhythmicPlaybackEngine(filename).run()


if __name__ == '__main__':
    main()
