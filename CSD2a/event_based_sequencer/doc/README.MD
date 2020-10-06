# Event Based Sequencer - Documentation

This document contains all documentation regarding the Event Based Sequencer assignment. The source code can be found in the ```src``` folder.

---

## Running
To make use of this project you need to run ```main.py``` via the console. The following assumes you are cd'ed into the ```../src``` folder:

```
python3 main.py
```

The console will now wait for you to give it commands. The following commands are valid:

|Command| Effect|
| --- | --- |
|```start```| starts playback |
|```stop```| stops playback |
|```rewind```| rewinds playhead (starts sequence from start) |
|```<number>``` | changes tempo (bpm) to given number |
|```load <file_path> as <sample_name>``` | loads given file so it can be used in the sequencer, addressed by the given name |
| ```remove <sample_name>``` | removes sample from sequencer and removes all events that used it |
| ```change``` | opens new menu to change the sequence |
| ```exit``` | saves the current state and exits the sequencer |


The ```change``` command opens a menu. In this menu you will be presented with the current sequence and the ability to change it. The following commands will be valid:
|Command | Effect|
|---|---|
|```s <sample_name> <bar> <beat> <tick>``` | adds event with given sample at given time stamp |
|```r <sample_name> <bar> <beat> <tick>``` | removes event with given sample and time stamp |
|```done``` | returns to main menu |

---
## Architecture
The sequencer is built off a couple of parts:

| Class Name | Task |
| --- | --- |
| ```Clock``` | makes sure each tick in the sequencer is exactly on time |
| ```EventHandler``` | handles a given event (plays sample), is also a ```SampleList.Listener```, so it knows when samples are added, so it can load them for playback |
| ```EventList``` | keeps track of all events, is also a ```SampleList.Listener``` so it can remove events when a sample is removed |
| ```SampleList``` | keeps track of all samples, notifies its listeners when samples are added or removed |
| ```SampleList.Listener``` | base class for a class that listens to a ```SampleList``` so it can update itself when a sample is added or removed |
| ```PlayHead``` | tracks the position (in ticks) within the sequence and wraps around if it's in looping mode |
| ```Sequencer``` | uses all the above classes to form a sequencer |
| ```TimeSignature``` | resembles the current time signature of the sequencer |
| ```ConsoleInterface``` | user interface (thread) using the console for interaction with a given ```Sequencer``` instance |
| ```IOProvider``` | base class for any io provider, to make the interface usable from different types of io, was intented to be used for testing only |

This architecture makes it possible to swap out different parts of the system. If for example we would want to handle events with an external sample player controlled via OSC, the only thing we need to do is making a new ```EventHandler``` and swap the ```SimpleAudio_EventHandler``` with this new one. This makes the system very modular and extendable.