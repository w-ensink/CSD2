cmake_minimum_required(VERSION 3.12)

project(hello_world VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} source/main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)