
# pull the Catch2 test library from github
FetchContent_Declare(Catch2
        GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
        GIT_TAG "166c5205984f3e7d269ec9894babc900da713077")

FetchContent_MakeAvailable(Catch2)

# function to add unit tests that link against all the required libs by default
function(add_unit_test NAME SOURCE)
    juce_add_console_app(${NAME})
    juce_generate_juce_header(${NAME})
    target_compile_features(${NAME} PRIVATE cxx_std_17)
    target_sources(${NAME} PRIVATE ${SOURCE})

    # add some definitions for juce
    target_compile_definitions(${NAME} PRIVATE JUCE_WEBBROWSER=0)

    # link the fetched libraries with our own executable
    target_link_libraries(${NAME} PRIVATE
            fmt::fmt
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_audio_devices
            Catch2WithMain
            ctre::ctre
            console_synth_lib
            butterworth_filter)

    # set the executable output directory to ./bin from the project root
    set_target_properties(${NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests")
endfunction()

add_unit_test(midi_message_queue_test midi_queue_test.cpp)
add_unit_test(melody_generator_test melody_generator_test.cpp)
add_unit_test(time_signature_test time_signature_test.cpp)
add_unit_test(midi_output_device_creation_test midi_output_device_creation_test.cpp)
add_unit_test(play_head_test play_head_test.cpp)
add_unit_test(sequencer_test sequencer_test.cpp)
add_unit_test(oscillator_test oscillator_test.cpp)
add_unit_test(adsr_test adsr_test.cpp)
add_unit_test(value_tree_test value_tree_test.cpp)