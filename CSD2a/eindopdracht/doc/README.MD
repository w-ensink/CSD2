# Irregular Beat Generator - Documentation

This document contains all documentation regarding the Irregular Beat Generator assignment. The source code can be found in the ```src``` folder.

---

## Running
To make use of this project you need to run ```main.py``` via the console. The following assumes you are cd'ed into the ```../src``` folder:

```
python3 main.py
```

<ins>Note that it is important to run the program from the src directory, because most sessions use relative file paths</ins>

The program lets you edit a sequence in realtime while also playing it back. (Almost) all changes are undoable and redoable. 
It can load new samples on the fly and has some useful algorithms for constructing sequences really fast. It's also fully adaptive. If you add an event that is located in a new bar (or if you change the time signature to one with fewer ticks per bar), the sequence will be extended an extra bar and the engine will adapt its looping point. 
When that event gets removed, it will go back to its original length.

Further more, you can save any session as json, so you can load it back in when you want to continue working on it. Exporting as MIDI is also possible (although it behaves a little weird at the moment).

Below is a list of valid commands. It will probably be extended in the future.

|Command|Effect|
|---|---|
|```play``` | starts playback|
|```stop``` | stops playback|
|```load <file_path> as <sample_name>```| loads sample at given file path (must be ```.wav```)|
|```remove <sample_name>``` | removes given sample from the session (and all the events that use it)|
|```r <sample_name> <bar> <beat> <tick>``` | removes event for given sample at given position|
|```s <sample_name> <bar> <beat> <tick>``` | adds event for given sample at given position|
|```clear [sample_name]```| removes all events [optionally for given sample only]|
|```euc <sample_name> <num_hits>```| makes euclidean distribution for sample with given number of events|
|```rr <sample_name> <num_ticks>```|rotates all events using given sample given number of ticks to the right|
|```rl <sample_name> <num_ticks>```|rotates all events using given sample given number of ticks to the left|
|```g``` or ```surprise me```| generates sequence |
|```sm <file_path>```|saves current session as midi (file_path must end with ```.mid``` or ```.midi```) |
|```sj <file_path>```|saves current session as json (file_path must end with ```.json```)|
|```lj <file_path>``` | loads session from json (file_path must end with ```.json```)|
|```tempo <tempo_bpm>``` | sets the tempo to the given argument|
|```ts <numerator>/<denominator>```| sets the time signature |
|```undo``` | undoes the last edit to the session|
|```redo``` | redoes the last undone edit to the session|
|```help```|shows a menu with a list of all valid commands|
|```exit```| exits the program|
