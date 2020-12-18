
add_library(butterworth_filter STATIC
        src/Biquad.cpp
        src/Butterworth.cpp)

target_include_directories(butterworth_filter PUBLIC include)

target_compile_features(butterworth_filter PRIVATE cxx_std_17)