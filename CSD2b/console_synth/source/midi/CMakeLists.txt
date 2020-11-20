

add_library(midi_lib STATIC
        midi_message_queue.h)

target_link_libraries(midi_lib PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        fmt::fmt)


target_compile_features(midi_lib PRIVATE cxx_std_17)

# get the parent directory to use as include directory
get_filename_component(INCLUDE_DIR ../ ABSOLUTE)
target_include_directories(midi_lib PUBLIC ${INCLUDE_DIR})

add_library(console_synth::midi_lib ALIAS midi_lib)