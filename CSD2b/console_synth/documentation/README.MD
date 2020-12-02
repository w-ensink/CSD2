# Console Synthesizer

### Building
You can build this project using CMake (version 3.17 or later)

To build it, do the following from the project directory ```/console_synth```.
```
mkdir build
cmake -S . -B build
cmake --build build -j <num_desired_threads>
```

The second step may take some time, since it needs to fetch a couple of libraries from github. 
This all goes automatically, so just let it do its thing. 
When the building is complete, you should have a working executable in the ```/bin``` directory it created for you.
To speedup the compilation time dramatically, I would recommend to fill in the total number 
of threads your computer has at ```<num_desired_threads>```

To build the tests as well, add an extra flag to the first CMake command:
```
mkdir build
cmake -S . -B build -DBUILD_TESTS=true
cmake --build build -j <num_desired_threads>
```

### Dependencies
This project uses the following C++ libraries:
- [JUCE](https://github.com/juce-framework/JUCE)
- [{fmt}](https://github.com/fmtlib/fmt)
- [CTRE](https://github.com/hanickadot/compile-time-regular-expressions)
- [Catch2](https://github.com/catchorg/Catch2)


All of them will be fetched from github automatically during the build process.
There might be some warning generated during the compilation process. On macOS Big Sur, 
all of them were related to JUCE's transition to CMake. None of them had any impact on the 
final build.

---
## Design

The general design consists of the following parts:
- Audio Playback (including synths)
- Sequencer (which takes a sequence and generates midi messages)
- Melody Generator
- Console Interface