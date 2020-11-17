

add_library(audio_lib STATIC
        audio_callback.h
        audio_callback.cpp
        noise_generator.h
        noise_generator.cpp)

target_compile_features("audio_lib" PRIVATE cxx_std_17)

target_include_directories("audio_lib" PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

target_link_libraries("audio_lib" PRIVATE
        fmt::fmt
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices)

# creates an alias for the library, to be more explicit about the name
add_library(console_synth::audio_lib ALIAS audio_lib)