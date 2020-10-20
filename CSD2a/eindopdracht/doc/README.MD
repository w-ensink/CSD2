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

Other classes can listen to a ```Session``` object by registering themselves as a listener (<ins>Observer Pattern</ins>). To be able to register themselves properly, they must inherit from ```Session.Listener```.
The only class in the system, for now, that needs to listen to the session, is the ```Sequencer```. This is because, as the session is altered by the user, the sequencer needs to update in realtime.

The user interface also needs to know about the changes, but since it is always redrawn after a command has been given by the user, it will find the changes anyway, so listening to the session is not needed.
If for some reason we want to listen from the UI in the future, that is still really easy to implement and doesn't change the engine part in any way.

The sequencer runs in its own thread, which gets started in its constructor. The sequencer works by repeating 3 simple steps over and over:
- Handle all events at the current time stamp
- Increment the playhead (move to the next time step)
- Wait (block) until the next tick

The ```PlayHead``` class is responsible for keeping track of the play head/current time stamp and the looping of the sequence. 
The ```Clock``` class knows how long to wait (tick time) and how long it has been since the last tick, so every time the events get handled is spot on. 

The sequencer also has a member called the event_handler, this can be set from the outside (dependency injection). Any class that derives from ```EventHandler``` can be used as the event handler. This means that, without changing anything else in the system, we could add event handlers that use midi or send event signals to Max via OSC. This is another great feature of the design that makes it really flexible and is an example of the <ins>Strategy Pattern</ins>.

Editing the engine's session object happens (only) via the ```SessionEditor```. The reason to keep this all in one place, is to keep all mutations at one location. That makes debugging way easier. This class uses the <ins>Facade Pattern</ins> to make a simplified editing interface for any UI class.

Now this is where the real fun begins. Every possible edit to the session is its own class. All of these classes derive from ```UndoableSessionEdit```. As the name suggest, these classes all have a "do" (in the code called ```perform()```) and an "undo" method. Everytime an edit is made, it gets pushed onto the undo stack in the ```SessionEditManager```. When the ```undo_last_edit()``` method on this manager gets called, the manager pops it off the undo stack, calls the ```undo()``` method of the edit class and pushes it onto the redo stack. When ```redo_last_undone_edit()``` gets called, it pops it of the redo stack, calls ```perform()``` and pushes it back on the undo stack. 
Since any action that derives from ```UndoableSessionEdit``` can be used, adding new edit options is trivial. Another feature that makes the system even more powerful and adaptive. The design is a combination of the <ins>Memento Pattern</ins> and the <ins>Command Pattern</ins>.

### User Interface
The engine is, as said before, completely independent of the UI. The UI, however, is dependent on the engine. It's not too bad here, because the UI is specific to the engine anyway. 
The UI uses almost the same pattern as the session editor, but without the saving of state (without the Memento part). The main ```ConsoleInterface``` class contains a bunch of command handlers. Each command handler is derived from ```CommandHandler```. Each derived class must implement ```matches_command()``` (which hould return if it can handle a given command), ```perform()``` (which should execute a given command) and ```get_help_string()``` (which should return a user readable string with info about the command). Again, this makes adding new commands very clean and self contained. 

## Rhythm Generation Algorithm
The algorithm used to generate rhythms makes heavy use of euclidean distributions. It picks the number of events it wants the lowest element to play randomly between 0 and (num_ticks_per_bar)/4 but it does it in a way that it has almost always have a kick, since it uses ceil to round the distribution. 
It does the same thing with the other instruments, but with different distribution probabilities. In addition to that, all instruments/except the kick also get rotated by some amount to make it more like a real beat (to avoid all instruments playing the 1 everytime).
The implementation can be found in ```session_editor.py``` as the ```GenerateSequence_SessionEdit```.

## Reflection
This first quarter of CSD, I've mainly focussed on the design and architecture of software. I though this was the area where I could improve the most. I've studied a bunch of design patterns, watched talks about best practices (one particularly good one is this [6 part series of clean code and clean architecture by Robert C. Martin (Uncle Bob)](https://www.youtube.com/watch?v=7EmboKQH8lM)) and spent a lot of time actually thinking about the design instead of just going to my keyboard and typing endlessly. This really helped me a lot. It surprised me how few bugs I've encountered in my program over the development time. It also feels like I was done quicker than you would expect with such a rich set of features. As described in the architecture part, I've used the following patterns in my code:
* Memento Pattern
* Command Pattern
* Observer Pattern
* Strategy Pattern
* Facade Pattern

My intention is to study and use all 23 of commonly used design pattern from the [design patterns book](https://en.wikipedia.org/wiki/Design_Patterns).

Further more, I've experimented a little bit with Test Driven Development. I've only done this on a couple of parts in the system. Earlier with one of the previous assignments, I've used it for the ```PlayHead``` and ```TimeSignature``` classes, and more recently I've used it on the ```Session``` class. The tests for that last one are still in the session.py file. The reason I've not yet done it on the full project, is because you need to have quite a bit of experience with it in order to scale it to big projects, and I didn't feel like I was there yet. As Uncle Bob said in his talk, you need to do a lot of small projects before you can really use TDD without messing up the whole structure of your program.

I found the experience of using TDD really satisfying and I want to get into it more during CSD2b. The feeling of being done when you're done is very nice. I don't have to waste my time debugging anything, since every single thing I wrote has already been tested when I'm done writing the code. Also getting that little shot of dopamine every time the test passes is great and refactoring is a breeze, since you don't have to worry about breaking the code. The tests will tell you immediately when you do so. If you quickly run the test after every change, you never get into big trouble.
