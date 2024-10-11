# Project Overview

This project demonstrates how to display the classic video "Bad Apple" using Windows window.

The project uses **OpenCV** to read the video file and process the frames, and **FFmpeg** to display the audio. The project is written in C++ and uses the Windows API to create the window.

This project can only be run on Windows.

# Directory Structure
```
bad_apple/
├─bad_apple.cpp
├─CMakeLists.txt
├─README.md
├─.vscode/
├─3rdparty/
│  ├─ffmpeg/
│  └─opencv/
├─doc/
├─prep/
│  └─prep.cpp
│  └─CMakeLists.txt
└─video/
   └─video.mp4
```

# Installation and Running
## Prerequisites

- CMake 3.10.0 or higher
- Windows OS x64
- OpenCV 4.10.0
- FFmpeg 6.0.1
- Bad Apple video file

In this project, there has already existed the [OpenCV](https://github.com/opencv/opencv) and [FFmpeg](https://ffmpeg.org) libraries built from source in the `3rdparty` directory.

If you want to use your own libraries, you can build them from source and replace the existing libraries.

You need to add `ffmpeg/bin` and `opencv/x64/mingw/bin` to the system environment variable `PATH`.

Remember to put the video file in the `video` directory. The video file should be named `video.mp4`. My video is downloaded from [here](https://www.youtube.com/watch?v=FtutLA63Cp8).

## Compilation and Running

There are two executables in this project: `bad_apple` and `prep`. Using windows powershell, you can compile and run the project as follows:

### Compile the `prep` executable:
```powershell
# In the root directory
cd prep
# Create a build directory
mkdir build
cd build
# Generate the build files
cmake -G "MinGW Makefiles" .
# Build the executable
cmake --build .
```

### Run the `prep` executable:

There are three parameters for the `prep` executable: `blockSize`, `maxBoxesInFrame`, and `whiteThreshold`. 

- The `blockSize` parameter is the size of the block in pixels. The smaller this value, the higher the analysis accuracy. The video frame of "bad apple" is 640x480, thus I recommend using a block size of 20.

- The `maxBoxesInFrame` parameter is the maximum number of boxes in a frame. The value limits the analysis accuracy.

- The `whiteThreshold` parameter is the threshold for the white color, which decides whether the boxes' target area is white or black in one frame. For example, if the value is 0.0, the boxes' target area is always black.

```powershell
# Run the executable
# .\prep.exe <blockSize> <maxBoxesInFrame> <whiteThreshold>
# e.g.
.\prep.exe 20 30 0.0
```

The result will be saved in `/doc/boxes.txt`, and the `bad_apple` executable will use this file to display the video.

### Check the frames

You can check the frames to debug the `prep` executable.

In the `prep` directory, there will be a `frames` directory containing the frames of the video.

There will be red boxes in the frames, which represent the target area of the boxes.

### Compile the `bad_apple` executable:
```powershell
# In the root directory
# Create a build directory
mkdir build
cd build
# Generate the build files
cmake -G "MinGW Makefiles" ..
# Build the executable
cmake --build .
```

### Run the `bad_apple` executable:

There is one parameter for the `bad_apple` executable: `scale`.

- The `scale` parameter decides the size of the area which windows will display. The value is the scale of the original video frame. For example, if the value is 1.0, the area will be 640x480.

```powershell
# Run the executable
# .\bad_apple.exe <scale>
# e.g.
.\bad_apple.exe 1.0
```

# Contribution

Feel free to contribute to this project. I coded this project in a simple way and just for fun. If you have any ideas to improve the project, please let me know.

# License

This project is licensed under the GNU General Public License v3.0. You can find the license in the `LICENSE` file. See the file for more details.