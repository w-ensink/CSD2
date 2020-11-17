# Console Synthesizer

### Building
You can build this project using CMake (version 3.17 or later)

To build it, do the following from the project directory (```console_synth```).
```
mkdir build
cmake -S . -B build
cmake --build build
```

The second step may take some time, since it needs to fetch a couple of libraries from github. This all goes automatically, so just let it do its thing. 

When the building is complete, you should have a working executable in the ```/bin``` directory it created for you.