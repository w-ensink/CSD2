cmake_minimum_required(VERSION 3.17)

project(console_synthesizer VERSION 1.0 LANGUAGES CXX)

# include the FetchContent cmake library to fetch external C++ libraries
include(FetchContent)

# Fetch fmtlib from github to make formatting text elegant
FetchContent_Declare(FmtLib
        GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
        GIT_TAG "6cdd1be93e2b8f8defa5e5faba7b93546cf4d1e7")

# Fetch juce from github to use as the audio backend
FetchContent_Declare(Juce
        GIT_REPOSITORY "https://github.com/juce-framework/JUCE.git"
        GIT_TAG "df3b49fbd35655ed2530e77e005034d87987c765")

FetchContent_Declare(Ctre
        GIT_REPOSITORY "https://github.com/hanickadot/compile-time-regular-expressions.git"
        GIT_TAG "9a37e550f1f2879b51582fba79a8bd7ba12cb5b0")

# Make all fetched libraries available inside the CMake project
FetchContent_MakeAvailable(Juce FmtLib Ctre)

# give the target a name (same as project name now)
set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(source)
add_subdirectory(apps)

# if you want to build the tests, invoke CMake with -DBUILD_TESTS=true
if (BUILD_TESTS)
    add_subdirectory(tests)
endif(BUILD_TESTS)