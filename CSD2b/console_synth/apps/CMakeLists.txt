
# add the executable target (juce requires that to be done through its own cmake api)
juce_add_console_app(${TARGET_NAME} PRODUCT_NAME synth VERSION 1.0)

# add source code to the target
target_sources(${TARGET_NAME} PRIVATE main.cpp)

# set the used C++ standard to C++17
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

# add some definitions for juce
target_compile_definitions(${TARGET_NAME} PRIVATE JUCE_WEBBROWSER=0)

# link the fetched libraries with our own executable
target_link_libraries(${TARGET_NAME} PRIVATE
        fmt::fmt
        juce::juce_core
        console_synth_lib
        ctre::ctre
        butterworth_filter)

# set the executable output directory to ./bin from the project root
set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
