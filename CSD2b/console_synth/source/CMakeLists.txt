
# add the executable target (juce requires that to be done through its own cmake api)
juce_add_console_app(${TARGET_NAME} PRODUCT_NAME synth VERSION 1.0)

# generate the JuceHeader.h for our project
juce_generate_juce_header(${TARGET_NAME})

# add source code to the target
target_sources(${TARGET_NAME} PRIVATE
        main.cpp
        format.h)

# set the used C++ standard to C++17
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

# add some definitions for juce
target_compile_definitions(${TARGET_NAME} PRIVATE JUCE_WEBBROWSER=0)

add_subdirectory(audio)

# link the fetched libraries with our own executable
target_link_libraries(${TARGET_NAME} PRIVATE
        fmt::fmt
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_audio_devices
        # internal libs
        console_synth::audio_lib
        )

# set the executable output directory to ./bin from the project root
set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
