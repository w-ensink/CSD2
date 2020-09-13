# Single Sample Sequencer - Documentation
This document includes all relevant documentation regarding the Single Sample Sequencer assignment.

---
## Running
### Sequencer
To make use of the project, you need to run ```main.py``` with via the command line, with one extra argument: the audio sample to play. The following example assumes you have cd'ed into the ```src``` folder.

```
python3 main.py ../audio/kick.wav
```
If the file you used doesn't exist or if it's not a ```.wav``` file, the program will tell you and then exit with an error code. You then have to try again with a different argument.

When running, you'll be presented with a command line interface. You can enter the following commands:
* ```start``` (to start the transport)
* ```stop``` (to stop the transport)
* ```rewind``` (to rewind the playhead to the start of the loop)
* ```<number>``` (to change the tempo)
* ```change``` (to enter a new menu that enables you to change the sequence)
* ```exit``` (to exit the program)

The menu to change the sequence should be pretty self explanatory, but keep in mind that the set/reset command arguments should be counting from 0, not from 1.

### Unit Tests
The project also contains a couple of unit tests, to test if everything works as expected. To run these, input the following into your terminal:
```
python3 unit_tests.py
```

## Architecture