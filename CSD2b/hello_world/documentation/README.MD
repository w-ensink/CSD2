# Hello, World! (C++)
Simple hello world program in C++, to build & run using make (from the ```hello_world``` directory):
```bash
$ make
$ cd build
$ ./hello_world
```
You can also build using CMake (from the ```hello_world``` directory):
```bash
$ mkdir build
$ cmake -S . -B build
$ cmake --build build
$ cd build
$ ./hello_world
```
Be careful with CMake, because since this project also has a makefile, it can override the handwritten makefile with a generated one. If you follow the steps above you should be fine though.