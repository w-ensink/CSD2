

add_library(audio_lib STATIC
        audio_processor_base.h
        audio_callback.h
        audio_callback.cpp
        noise_generator.h
        noise_generator.cpp
        sine_synthesizer.h
        oscillator.h
        audio_engine.h
        audio_engine.cpp
        saw_synthesizer.h reverb.h)

target_compile_features("audio_lib" PRIVATE cxx_std_17)


# get the parent directory to use as include directory
get_filename_component(INCLUDE_DIR ../ ABSOLUTE)
target_include_directories("audio_lib" PUBLIC "${INCLUDE_DIR}")


target_link_libraries("audio_lib" PRIVATE
        fmt::fmt
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors)

# creates an alias for the library, to be more explicit about the name
add_library(console_synth::audio_lib ALIAS audio_lib)
