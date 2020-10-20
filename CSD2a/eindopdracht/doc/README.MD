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
|```sp <sample_name> <low|mid|high>```|sets the spectral position for given sample (important for generation and midi saving)|
|```tempo <tempo_bpm>``` | sets the tempo to the given argument|
|```ts <numerator>/<denominator>```| sets the time signature |
|```undo``` | undoes the last edit to the session|
|```redo``` | redoes the last undone edit to the session|
|```help```|shows a menu with a list of all valid commands|
|```exit```| exits the program|

---
## Architecture & Design
The architecture of the system has been optimized to make adding new features very simple. The techniques used to accomplish this will be described below.

The system consists of two main parts: the user interface and the engine. The engine is completely independent of the user interface. It does not know a single thing about it. This is great, because changing the user interface happens for totally different reasons than changing the core sequencer logic. If for example we would want a graphical user interface instead of the console one we have now, we could write one without touching the engine at all. 


### Engine
The main class of the engine is called ```Engine```. This class' main purpose it to combine all elements that make up the sequencer, editor, exporter and importer functionalities. This class also contains the ```Session``` object, which contains the session information: the tempo, time signature, samples and events.

Other classes can listen to a Session object by registering themselves as a listener (Observer Pattern). To be able to register themselves properly, they must inherit from ```Session.Listener```.
The only class in the system, for now, that needs to listen to the session, is the ```Sequencer```. This is because, as the session is altered by the user, the sequencer needs to update in realtime.

The user interface also needs to know about the changes, but since it is always redrawn after a command has been given by the user, it will find the changes anyway, so listening to the session is not needed.
If for some reason we want to listen from the UI in the future, that is still really easy to implement and doesn't change the engine part in any way.

The sequencer runs in its own thread, which gets started in its constructor. The sequencer works by repeating 3 simple steps over and over:
- Handle all events at the current time stamp
- Increment the playhead (move to the next time step)
- Wait (block) until the next tick

The ```PlayHead``` class is responsible for keeping track of the play head/current time stamp and the looping of the sequence. 
The ```Clock``` class knows how long to wait (tick time) and how long it has been since the last tick, so every time the events get handled is spot on. 

The sequencer also has a member called the event_handler, this can be set from the outside (dependency injection). Any class that derives from ```EventHandler``` can be used as the event handler. This means that, without changing anything else in the system, we could add event handlers that use midi or send event signals to Max via OSC. This is another great feature of the design that makes it really flexible. 

Editing the engine's session object happens (only) via the ```SessionEditor```. The reason to keep this all in one place, is to keep all mutations at one location. That makes debugging way easier. 

Now this is where the real fun begins. Every possible edit to the session is its own class. All of these classes derive from ```UndoableSessionEdit```. As the name suggest, these classes all have a "do" (in the code called "perform()") and an "undo" method. Everytime an edit is made, it gets pushed onto the undo stack in the ```SessionEditManager```. When the undo() method on this manager gets called, the manager pops it off the undo stack, calls the undo method and pushes it onto the redo stack. When redo() gets called, it pops it of the redo stack, calls perform() and pushes it back on the undo stack. 
Since any action that derives from ```UndoableSessionEdit``` can be used, adding new edit options is trivial. Another feature that makes the system even more powerful and adaptive. The design is a combination of the Memento Pattern and the Command Pattern.

### User Interface
The engine is, as said before, completely independent of the UI. The UI, however, is dependent on the engine. It's not too bad here, because the UI is specific to the engine anyway. 
The UI uses almost the same pattern as the session editor, but without the saving of state (without the Memento part). The main ```ConsoleInterface``` class contains a bunch of command handlers. Each command handler is derived from ```CommandHandler```. Each derived class must implement ```matched_command()``` (which hould return if it can handle a given command), ```perform()``` (which should execute a given command) and ```get_help_string()``` (which should return a user readable string with info about the command). Again, this makes adding new commands very clean and self contained. 

## Reflection
