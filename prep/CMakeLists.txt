cmake_minimum_required(VERSION 3.10)
project(prep)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set OpenCV directory
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/../3rdparty/opencv")

# Set OpenCV include directory
include_directories("${OpenCV_DIR}/include")

# Find OpenCV
find_package(OpenCV REQUIRED)

# Link OpenCV library
link_directories("${OpenCV_DIR}/x64/mingw/lib")
link_directories("${OpenCV_DIR}/x64/mingw/bin")

# Add executable
add_executable(prep prep.cpp)

# Link OpenCV library
target_link_libraries(prep ${OpenCV_LIBS})