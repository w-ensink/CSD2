# Python Basics - Documentation
This file explains how to use the scripts that can be found in the ```../src``` folder.
This folder contains two files, namely ```hello_world.py``` and ```playsound.py```. These are both standalone Python 3 projects.

---
## Hello, World!
```hello_world.py``` can be run by simply using: ```python3 hello_world.py``` assuming your terminal has been cd'ed to the ```src``` directory.

The program should first output "Hello, World!" and then ask for your name. It should finally print "Hello, [your name]!" to the console and exit

---
## Play Sound
```playsound.py``` expects one command line parameter, namely the relative path to the file to play. 
For example: ```python3 playsound.py ../audio/kick.wav```, assuming your terminal has been cd'ed into the src directory.

If the file doesn't exist or isn't a ```.wav``` file, the program will tell you so and then exit. 
If it does exist, it will ask you how many times you want to play it and then play it that many times (and wait for it to be done). It should then ask if you want to give it another go. 
All user interaction will be checked on how valid they are and if they're invalid, the program should tell you why and then ask again.
