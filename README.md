# Game Engine

## Windows

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_OPENGL=ON
-DBACKEND_DIRECTX=ON
```

Build and run
```bash
win-build-directx.bat
win-build-opengl.bat
```

## MacOS

Prerequisites
```bash
brew install cmake
# If using OpenGL backend
brew install glfw
brew install glew
```

Run with one of the following CMake options to choose a backend
```bash
-DBACKEND_METAL=ON
-DBACKEND_OPENGL=ON
```

Build and run
```bash
mkdir build
cd build
cmake .. -DBACKEND_METAL=ON
make
```

## iOS

Prerequisites
```bash
brew install cmake
```

Build
```bash
./ios-build.sh
```

## Linux

Prerequisites
```bash
sudo apt-get update
sudo apt-get install -y cmake libglfw3-dev libglew-dev
```

Build and run
```bash
mkdir build
cd build
cmake ..
make
./GameEngine
```
