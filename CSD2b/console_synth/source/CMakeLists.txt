
add_library(console_synth_lib STATIC
        audio/audio_callback.cpp
        audio/audio_engine.cpp
        audio/noise_generator.cpp
        # sequencer
        sequencer/time_signature.cpp)

target_link_libraries(console_synth_lib PRIVATE
        butterworth_filter
        fmt::fmt
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors)

target_include_directories(console_synth_lib PUBLIC "../include")

target_compile_features(console_synth_lib PRIVATE cxx_std_17)