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
- [Butterworth-Filter-Design](https://github.com/ruohoruotsi/Butterworth-Filter-Design)


Almost all of them will be fetched from github automatically during the build process.
There might be some warning generated during the compilation process. On macOS Big Sur, 
all of them were related to JUCE's transition to CMake. None of them had any impact on the 
final build.
The Butterworth-Filter-Design library is included directly into the ```external``` directory, because it doesn't have CMake support by itself.
To incorporate it into this project correctly I needed to do some extra CMake configuration to build it as a static library.

---
## Oscillator Design
The oscillator library I made is heavily templated. This makes it really modular,
while giving the compiler a lot to work with. The library is centered around one 
simple interface that should be the same for any oscillator. The main reason to make 
the oscillators compile time polymorphic, is performance. Since for any oscillator the 
advance() method gets called for every new sample, the overhead of virtual dispatch can be pretty high. 
With the compiler knowing the entire flow at compile time, it can inline anything it wants and therefore 
do way more optimisation.
 
I have done a benchmarking test with two fm oscillators 
with exactly the same configurations (both had 3 modulators and the same ratio's and indices).
All the logic inside the render functions was the same. The only difference was that one
was my template library and the other used runtime polymorphism with virtual functions.
The results showed that my template oscillator was about 275 times faster.

There are a couple of primitive naive oscillator types (square, saw, sine, triangle, noise, pulse...),
that can then be wrapped inside a higher level 'shell' to add functionality (like anti aliasing) 
or can be used to make higher level oscillators, like FM, AM and RM oscillators. 

```cpp
// simple, naive square wave oscillator (with heavy aliasing)
auto osc = SquareOsc<float>();

// anti aliased square wave oscillator with 8x oversampling
auto osc = AntiAliased<SquareOsc<float>, 8>();

// fm oscillator with sine carrier and saw modulator (at double sample precision)
auto osc = FmOsc<SineOsc<double>, SawOsc<double>>();

// anti aliased fm oscillator (8x oversampling) with sine carrier, one triangle modulator 
// (which is itself ring modulated by a saw) 
// and one anti aliased square modulator with 4x oversampling
auto osc = AntiAliased<FmOsc<SineOsc<float>, 
                             RmOsc<TriangleOsc<float>, SawOsc<float>>,
                             AntiAliased<SquareOsc<float>, 4>>, 8>();
```

The types of primitives currently available are:
* SineOsc
* SawOsc
* SquareOsc
* NoiseOsc
* TriangleOsc

The types of higher order oscillators currently available are:
* FmOsc
* AmOsc
* RmOsc

The types of oscillator wrappers:
* AntiAliased