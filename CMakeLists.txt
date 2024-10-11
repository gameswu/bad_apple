cmake_minimum_required(VERSION 3.10)
project(bad_apple)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)

# 添加可执行文件
add_executable(bad_apple bad_apple.cpp)

# 设置编译选项以优化速度
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(bad_apple PRIVATE -O3 -march=native)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(bad_apple PRIVATE /O2 /arch:AVX2)
endif()