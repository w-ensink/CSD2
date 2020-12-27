
add_library(console_synth_lib STATIC
        engine.cpp
        #utility
        utility/scoped_message_thread_enabler.cpp
        # audio
        audio/audio_callback.cpp
        # sequencer
        sequencer/time_signature.cpp
        # console_interface
        console_interface/console_interface.cpp)

target_link_libraries(console_synth_lib PRIVATE
        butterworth_filter
        fmt::fmt
        ctre::ctre
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors)

target_include_directories(console_synth_lib PUBLIC "../include")

target_compile_features(console_synth_lib PRIVATE cxx_std_17)