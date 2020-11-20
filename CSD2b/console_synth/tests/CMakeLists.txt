
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
            Catch2WithMain)

    # set the executable output directory to ./bin from the project root
    set_target_properties(${NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests")
endfunction()

# add cli unit test
add_unit_test(cli_test cli_test.cpp)

# add midi message queue unit test (and link with internal midi lib)
add_unit_test(midi_message_queue_test midi_queue_test.cpp)
target_link_libraries(midi_message_queue_test PRIVATE console_synth::midi_lib)