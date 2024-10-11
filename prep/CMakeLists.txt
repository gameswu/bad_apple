cmake_minimum_required(VERSION 3.10)
project(Video2Image)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)

# 设置OpenCV目录
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/../3rdparty/opencv")

# 手动设置OpenCV包含路径
include_directories("${OpenCV_DIR}/include")

# 查找OpenCV库
find_package(OpenCV REQUIRED)

# 手动设置OpenCV库路径
link_directories("${OpenCV_DIR}/x64/mingw/lib")
link_directories("${OpenCV_DIR}/x64/mingw/bin")

# 添加可执行文件
add_executable(prep prep.cpp)

# 链接OpenCV库
target_link_libraries(prep ${OpenCV_LIBS})