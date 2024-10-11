cmake_minimum_required(VERSION 3.10)
project(bad_apple)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add executable
add_executable(bad_apple bad_apple.cpp)

# Set compiler flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(bad_apple PRIVATE -O3 -march=native)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(bad_apple PRIVATE /O2 /arch:AVX2)
endif()