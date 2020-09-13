# Rhythmic Playback - Documentation
This document includes all documentation for the Rhythmic Playback assignment. The source code can be found in the ```src``` folder.

---
## Running 
To make use of the project, you need to run ```main.py``` with via the command line, with one extra argument: the audio sample to play. The following example assumes you have cd'ed into the ```src``` folder.
``` bash
python3 main.py ../audio/kick.wav
```
If the file you used doesn't exist or if it's not a ```.wav``` file, the program will tell you and then exit with an error code. You then have to try again with a different argument.

If a valid file has been chosen, the program should ask you how many notes are in the rhythm you want to play. After entering a number, it should ask you n number of times how long (in quarter notes) it should wait. It should finally ask for the tempo (bpm) at which you want to play back your sequence. After all input it should play the sequence and then ask if you want to play another one.

---
## Architecture
The architecture is built from a couple of components, each designed to take on one task.
For most tasks a universal interface is made to make the system more extendable. 
For example: the ```Sequencer``` class only knows it has a ```SamplePlayer```. A ```SamplePlayer``` is then defined as a class with at least the member function ```trigger()``` which should play the sound in a non-blocking way. This way the sequencer can interact with any class that derives from ```SamplePlayer```. For this project, we make use of an implementation called ```SimpleAudio_SamplePlayer```, which is an implementation of ```SamplePlayer``` that uses the ```simpleaudio``` Python module. This way the system stays open for extension, but closed for modification.

As a whole, the system is made of 4 components:
* ```InputProvider```: Responsable for getting and parsing user input as a ```RhythmDescription```.
* ```RhythmicPlaybackEngine```: Responsible for the bigger picture (putting all elements together).
* ```RhythmToSequenceConverter```: Responsible for translating rhythm input from the user into a format the sequencer understands.
* ```Sequencer```: Responsible for playing a sequence (triggering a sampler at the right times)
* ```SamplePlayer```: Responsible for playing a sample in a non-blocking way.

These classes communicate with 2 different data structures, containing all relevant info:
* ```RhythmDescription```: a structure holding rhythm info like the intervals in beats and the tempo in BPM.
* ```SequenceDescription```: a structure holdong info that is useful for the sequencer, which in this case is only the time intervals.