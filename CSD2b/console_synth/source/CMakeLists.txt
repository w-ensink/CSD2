
add_library(console_synth STATIC
        audio/audio_callback.cpp
        audio/audio_engine.cpp
        audio/noise_generator.cpp)

target_link_libraries(console_synth PRIVATE
        fmt::fmt
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors)


target_include_directories(console_synth PUBLIC "../include")

target_compile_features(console_synth PRIVATE cxx_std_17)